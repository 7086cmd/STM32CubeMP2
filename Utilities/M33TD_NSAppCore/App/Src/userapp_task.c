/**
    ******************************************************************************
    * @file    userapp_task.c
    * @author  MCD Application Team
    * @brief   User application task implementation file.
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

/**
    * @addtogroup UserAppTask
    * @{
    */

/* Includes ------------------------------------------------------------------*/
#include "userapp_task.h"
#include "led_driver.h"

/**
    * @brief Handle for the User Application Task thread.
    */
static osThreadId_t UserAppTaskHandle;

/**
    * @brief Attributes for the User Application Task thread.
    */
static const osThreadAttr_t UserAppTaskAttr =
{
    .name = "UserAppTask",
    .priority = (osPriority_t)USERAPP_TASK_PRIORITY,
    .stack_size = USERAPP_TASK_STACK_SIZE
};

/**
  * @brief  User Application Task thread function.
  * @param  argument: Not used.
  */
static void UserAppTask(void *argument)
{
    (void)argument;

    (void)led_driver.init();

    for (;;)
    {
        led_driver.led_toggle(LED_ID_3);
        osDelay(1000);
    }
}

/**
  * @brief  Initialize the User Application Task and its resources.
  *         Creates the task thread and initializes required peripherals.
  */
void UserAppTask_Init(void)
{
    UserAppTaskHandle = osThreadNew(UserAppTask, NULL, &UserAppTaskAttr);
    if (UserAppTaskHandle == NULL)
    {
        NSAppCore_ErrorHandler();
    }
}

/**
    * @brief  De-initialize the User Application Task and its resources.
    *         Terminates the task thread and de-initializes peripherals.
    */
void UserAppTask_DeInit(void)
{
    if (UserAppTaskHandle != NULL)
    {
        osThreadTerminate(UserAppTaskHandle);
        UserAppTaskHandle = NULL;
    }

    led_driver.deinit();
}

/**
    * @}
    */

