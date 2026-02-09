/**
 ******************************************************************************
 * @file   backtrace.c
 * @author MCD Application Team
 * @brief  print backtrace with gcc unwind functions
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
#include "backtrace.h"
#include "fault_config.h"
#if FAULT_EXCEPTION_ENABLE && FAULT_EXCEPTION_BACKTRACE_ENABLE
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unwind.h>

#define MAX_STACK_FRAMES FAULT_MAX_BACKTRACE_DEPTH

typedef struct {
    uint32_t buffer[MAX_STACK_FRAMES];
    int n_frames;
} bkp_trace_t;

static _Unwind_Reason_Code trace_func(_Unwind_Context *context, void *arg)
{
    bkp_trace_t *trace = arg;
    uint32_t ret_addr;

    if (trace->n_frames > MAX_STACK_FRAMES)
        return _URC_END_OF_STACK;

    ret_addr = _Unwind_GetIP(context);
    if (ret_addr == 0)
        return _URC_END_OF_STACK;

    // set caller addr
    ret_addr -= 4;

    if (trace->n_frames > 0 && trace->buffer[trace->n_frames - 1] == ret_addr)
        return _URC_END_OF_STACK;

    trace->buffer[trace->n_frames++] = ret_addr;

    return _URC_NO_REASON;
}

void BACKTRACE_Print(void)
{
    bkp_trace_t bk_trace = { .n_frames = 0 };
    int i;

    _Unwind_Backtrace(trace_func, &bk_trace);

    if (bk_trace.n_frames == 0)
        return;

    FAULT_LOG("Backtrace:");
    for (i = 0; i < bk_trace.n_frames && i < MAX_STACK_FRAMES; i++)
        FAULT_LOG("  Frame: %02d: 0x%08lx", i, bk_trace.buffer[i]);
}
#else
void BACKTRACE_Print(void) { /* No-op if disabled */ }
#endif

