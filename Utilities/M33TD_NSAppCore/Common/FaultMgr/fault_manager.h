/**
 ******************************************************************************
 * @file   fault_manager.h
 * @author MCD Application Team
 * @brief  Fault management API for exception and backtrace handling
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
#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#include "fault_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void FAULT_Init(void);
void FAULT_RegisterCallback(FAULT_Callback_t cb);

#ifdef __cplusplus
}
#endif

#endif // FAULT_MANAGER_H

