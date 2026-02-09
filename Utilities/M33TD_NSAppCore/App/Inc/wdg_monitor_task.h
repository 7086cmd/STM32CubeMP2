/**
  ******************************************************************************
  * @file    wdg_monitor_task.h
  * @author  MCD Application Team
  * @brief   Header file for Watchdog Monitor Task implementation.
  ******************************************************************************
  * @attention
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

#ifndef WDG_MONITOR_TASK_H
#define WDG_MONITOR_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "nsappcore_config.h"

/**
  * @brief  Initialize the Watchdog Monitor Task and its resources.
  * @retval None
  */
void WdgMonitorTask_Init(void);

/**
  * @brief  De-initialize the Watchdog Monitor Task and release its resources.
  * @retval None
  */
void WdgMonitorTask_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* WDG_MONITOR_TASK_H */

