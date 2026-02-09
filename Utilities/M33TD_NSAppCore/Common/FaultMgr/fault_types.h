/**
 ******************************************************************************
 * @file   fault_types.h
 * @author MCD Application Team
 * @brief  Fault context and type definitions for exception handling
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
#ifndef FAULT_TYPES_H
#define FAULT_TYPES_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    FAULT_TYPE_HARDFAULT,
    FAULT_TYPE_MEMMANAGE,
    FAULT_TYPE_BUSFAULT,
    FAULT_TYPE_USAGEFAULT,
    FAULT_TYPE_UNKNOWN
} fault_type_t;

typedef struct {
    uint32_t r0, r1, r2, r3, r12, lr, pc, psr;
} stack_frame_t;

typedef struct {
    uint32_t r4, r5, r6, r7, r8, r9, r10, r11;
} stack_callee_t;

typedef struct {
    uint32_t icsr, shcrs, cfsr, hfsr, mmfar, bfar;
} scb_t;

typedef struct {
    uint32_t msp, psp, exc_return, control;
    uintptr_t frame_ptr;
    stack_frame_t stack_frame;
    stack_callee_t stack_callee;
    scb_t scb;
    fault_type_t type;
} fault_info_t;

typedef void (*FAULT_Callback_t)(const fault_info_t *info);

#endif // FAULT_TYPES_H

