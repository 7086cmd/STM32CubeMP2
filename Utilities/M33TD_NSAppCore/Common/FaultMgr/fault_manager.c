/**
 ******************************************************************************
 * @file   fault_manager.c
 * @author MCD Application Team
 * @brief  faults exception with extra information for debug
 ******************************************************************************
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "fault_manager.h"
#include "fault_config.h"
#if FAULT_EXCEPTION_ENABLE
#include <string.h>
#include <cmsis.h>
#include <inttypes.h>
#include "backtrace.h"

static fault_info_t fault;
static FAULT_Callback_t user_cb = NULL;

__WEAK void FAULT_Init(void) {
    // Default: Enable Bus, Mem, Usage faults and division-by-zero trap
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                | SCB_SHCSR_BUSFAULTENA_Msk
                | SCB_SHCSR_MEMFAULTENA_Msk;
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
}

void FAULT_RegisterCallback(FAULT_Callback_t cb) {
    user_cb = cb;
}

static inline void scb_dump(scb_t *scb) {
    scb->icsr = SCB->ICSR;
    scb->shcrs = SCB->SHCSR;
    scb->cfsr = SCB->CFSR;
    scb->hfsr = SCB->HFSR;
    scb->mmfar = SCB->MMFAR;
    scb->bfar = SCB->BFAR;
}

static void _register_print(uintptr_t frame_ptr, stack_frame_t *frame, stack_callee_t *callee) {
    FAULT_LOG("Exception frame: 0x%08" PRIxPTR, frame_ptr);
    FAULT_LOG("  r0:   0x%lx", frame->r0);
    FAULT_LOG("  r1:   0x%lx", frame->r1);
    FAULT_LOG("  r2:   0x%lx", frame->r2);
    FAULT_LOG("  r3:   0x%lx", frame->r3);
    FAULT_LOG("  r4:   0x%lx", callee->r4);
    FAULT_LOG("  r5:   0x%lx", callee->r5);
    FAULT_LOG("  r6:   0x%lx", callee->r6);
    FAULT_LOG("  r7:   0x%lx", callee->r7);
    FAULT_LOG("  r8:   0x%lx", callee->r8);
    FAULT_LOG("  r9:   0x%lx", callee->r9);
    FAULT_LOG("  r10:  0x%lx", callee->r10);
    FAULT_LOG("  r11:  0x%lx", callee->r11);
    FAULT_LOG("  r12:  0x%lx", frame->r12);
    FAULT_LOG("  lr:   0x%lx", frame->lr);
    FAULT_LOG("  pc:   0x%lx", frame->pc);
    FAULT_LOG("  xpsr: 0x%lx", frame->psr);
}

static void _scb_print(scb_t *scb) {
#if FAULT_VERBOSE_SCB
    uint32_t vectactive = scb->icsr & 0x1FF;
    uint32_t mmfsr = (scb->cfsr >> 0) & 0xFF;
    uint32_t bfsr = (scb->cfsr >> 8) & 0xFF;
    uint32_t ufsr = (scb->cfsr >> 16) & 0xFFFF;
    const char *exc_name = NULL;
    switch (vectactive) {
    case 3: exc_name = "HardFault"; break;
    case 4: exc_name = "MemManage"; break;
    case 5: exc_name = "BusFault"; break;
    case 6: exc_name = "UsageFault"; break;
    default: exc_name = NULL; break;
    }
    if (exc_name)
        FAULT_LOG("Exception number: %ld (%s)", vectactive, exc_name);
    else
        FAULT_LOG("Exception number: %ld", vectactive);
    FAULT_LOG("Fault status hfsr: 0x%lx", scb->hfsr);
    if (scb->hfsr & (1UL << 30)) FAULT_LOG("  -> Forced HardFault");
    FAULT_LOG("Fault status cfsr: 0x%lx", scb->cfsr);
    if (mmfsr) {
        FAULT_LOG("  Mem fault:");
        if (scb->cfsr & (1UL << 0)) FAULT_LOG("    -> Instruction access violation");
        if (scb->cfsr & (1UL << 1)) FAULT_LOG("    -> Data access violation");
        if (scb->cfsr & (1UL << 3)) FAULT_LOG("    -> on unstacking for a return from exception");
        if (scb->cfsr & (1UL << 4)) FAULT_LOG("    -> on stacking for exception entry");
        if (scb->cfsr & (1UL << 5)) FAULT_LOG("    -> occurred during floating-point");
        if (scb->cfsr & (1UL << 7)) FAULT_LOG("    -> address: 0x%lx", scb->mmfar);
    }
    if (bfsr) {
        FAULT_LOG("  Bus fault:");
        if (scb->cfsr & (1UL << 8)) FAULT_LOG("    -> Instruction bus error");
        if (scb->cfsr & (1UL << 9)) FAULT_LOG("    -> Precise data bus error");
        if (scb->cfsr & (1UL << 11)) FAULT_LOG("    -> on unstacking for a return from exception");
        if (scb->cfsr & (1UL << 12)) FAULT_LOG("    -> on stacking for exception entry");
        if (scb->cfsr & (1UL << 13)) FAULT_LOG("    -> occurred during floating-point");
        if (scb->cfsr & (1UL << 15)) FAULT_LOG("    -> address: 0x%lx", scb->bfar);
    }
    if (ufsr) {
        FAULT_LOG("  Usage fault:");
        if (scb->cfsr & (1UL << 16)) FAULT_LOG("    -> Undefined instruction");
        if (scb->cfsr & (1UL << 17)) FAULT_LOG("    -> Invalid state flag");
        if (scb->cfsr & (1UL << 18)) FAULT_LOG("    -> Invalid PC flag");
        if (scb->cfsr & (1UL << 19)) FAULT_LOG("    -> No coprocessor flag");
        if (scb->cfsr & (1UL << 24)) FAULT_LOG("    -> Stack overflow");
        if (scb->cfsr & (1UL << 25)) FAULT_LOG("    -> Unaligned access flag");
        if (scb->cfsr & (1UL << 28)) FAULT_LOG("    -> Divide by zero");
    }
#else
    FAULT_LOG("Exception number: %ld", scb->icsr & 0x1FF);
    FAULT_LOG("HFSR: 0x%lx, CFSR: 0x%lx", scb->hfsr, scb->cfsr);
#endif
}

static void fault_print(fault_info_t *f) {
    _scb_print(&f->scb);
    FAULT_LOG("Exception context:");
    FAULT_LOG("  exc_return (lr): 0x%lx", f->exc_return);
    FAULT_LOG("  MSP: 0x%lx", f->msp);
    FAULT_LOG("  PSP: 0x%lx", f->psp);
    if (f->exc_return & (1UL << 3))
        FAULT_LOG("  thread mode");
    else
        FAULT_LOG("  handler mode");
    _register_print(f->frame_ptr, &f->stack_frame, &f->stack_callee);
}

void FAULT_Info(uint32_t MSP_in, uint32_t PSP_in, uint32_t LR_in, uint32_t *callee_saved) {
    fault.msp = MSP_in;
    fault.psp = PSP_in;
    fault.exc_return = LR_in;
    fault.control = __get_CONTROL();
    if (fault.control & (1UL << 1))
        fault.frame_ptr = (uintptr_t)PSP_in;
    else
        fault.frame_ptr = (uintptr_t)MSP_in;
    memcpy(&(fault.stack_frame), (uint32_t *)(fault.frame_ptr), sizeof(fault.stack_frame));
    if (callee_saved)
        memcpy(&(fault.stack_callee), callee_saved, sizeof(fault.stack_callee));
    scb_dump(&fault.scb);
    fault_print(&fault);
#if FAULT_EXCEPTION_BACKTRACE_ENABLE
    BACKTRACE_Print();
#endif
    if (user_cb) user_cb(&fault);
}

#define FAULT_INFO_ASM() \
    __ASM volatile( \
        "MRS    R0, MSP\n" \
        "MRS    R1, PSP\n" \
        "PUSH   {R4-R11}\n" \
        "MOV    R2, LR\n" \
        "MOV    R3, SP\n" \
        "BL     FAULT_Info\n" \
        "ADD    SP, #32\n" \
    )

#define FAULT_PANIC() FAULT_PANIC_HANDLER()

__attribute__((naked)) void HardFault_Handler(void) {
    FAULT_INFO_ASM();
    FAULT_PANIC();
}
__attribute__((naked)) void MemManage_Handler(void) {
    FAULT_INFO_ASM();
    FAULT_PANIC();
}
__attribute__((naked)) void BusFault_Handler(void) {
    FAULT_INFO_ASM();
    FAULT_PANIC();
}
__attribute__((naked)) void UsageFault_Handler(void) {
    FAULT_INFO_ASM();
    FAULT_PANIC();
}
#endif // FAULT_ENABLE

