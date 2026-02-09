/**
 ******************************************************************************
 * @file   fault_config.h
 * @author MCD Application Team
 * @brief  Configuration macros for fault and backtrace management
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
#ifndef FAULT_CONFIG_H
#define FAULT_CONFIG_H

#include <stdio.h>

// Enable/disable the entire fault mechanism
#ifndef FAULT_EXCEPTION_ENABLE
#define FAULT_EXCEPTION_ENABLE            1
#endif

// Enable/disable backtrace support
#ifndef FAULT_EXCEPTION_BACKTRACE_ENABLE
#define FAULT_EXCEPTION_BACKTRACE_ENABLE  1
#endif

// Set maximum backtrace depth
#ifndef FAULT_MAX_BACKTRACE_DEPTH
#define FAULT_MAX_BACKTRACE_DEPTH 15
#endif

// Enable/disable verbose SCB output
#ifndef FAULT_VERBOSE_SCB
#define FAULT_VERBOSE_SCB       1
#endif

#define FAULT_COLOR_RED   "\x1b[31m"
#define FAULT_COLOR_BOLD  "\x1b[1m"
#define FAULT_COLOR_RESET "\x1b[0m"

#ifndef FAULT_LOG
#define FAULT_LOG(fmt, ...) \
    printf(FAULT_COLOR_BOLD FAULT_COLOR_RED "[NS] [FAULT] " fmt FAULT_COLOR_RESET "\r\n", ##__VA_ARGS__)
#endif

#ifndef FAULT_PANIC_HANDLER
#define FAULT_PANIC_HANDLER()    while(1){}
#endif

#endif // FAULT_CONFIG_H

