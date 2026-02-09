/**
  ******************************************************************************
  * @file    btn_monitor_task.h
  * @author  MCD Application Team
  * @brief   Header file for Button Monitor Task implementation.
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

#ifndef BTN_MONITOR_TASK_H
#define BTN_MONITOR_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "nsappcore_config.h"

typedef enum {
    BUTTON_EVENT_SHORT_PRESS = 0,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_VERY_LONG_PRESS,
    BUTTON_EVENT_TYPE_COUNT
} ButtonEventType_t;

typedef void (*ButtonEventCallback_t)(ButtonEventType_t event, void *context);

/**
  * @brief Register a callback for a specific button event type.
  * @param eventType Button event type to listen for.
  * @param cb Callback function to register.
  * @param context User context pointer passed to the callback.
  * @retval 0 on success, -1 on invalid type, -2 if already registered.
  */
int BtnMonitorTask_RegisterListener(ButtonEventType_t eventType, ButtonEventCallback_t cb, void *context);

/**
  * @brief Unregister a previously registered callback for a button event type.
  * @param eventType Button event type to stop listening for.
  * @retval 0 on success, -1 on invalid type, -2 if not registered.
  */
int BtnMonitorTask_UnregisterListener(ButtonEventType_t eventType);

/**
  * @brief Initialize the Button Monitor Task and its resources.
  */
void BtnMonitorTask_Init(void);

/**
  * @brief De-initialize the Button Monitor Task and release its resources.
  */
void BtnMonitorTask_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* BTN_MONITOR_TASK_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

