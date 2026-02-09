/**
  ******************************************************************************
  * @file    openamp_task.h
  * @author  MCD Application Team
  * @brief   Thread Openamp header file
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

#ifndef OPENAMP_TASK_H
#define OPENAMP_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "nsappcore_config.h"

/**
  * @brief  Enumeration for OpenAMP task commands.
  */
typedef enum
{
  OPENAMP_CMD_INIT = 0,
  OPENAMP_CMD_RX_EVENT,
  OPENAMP_CMD_REINIT,
  OPENAMP_CMD_SHUTDOWN,
  OPENAMP_CMD_REBOOT,
} OpenampCommand_t;

/**
  * @brief OpenAMP task command.
  */
typedef struct
{
  OpenampCommand_t type;
  uint32_t param;
} OpenampTaskCommand_t;

/**
  * @brief Initialize the OpenAMP task thread and resources.
  *        Creates the OpenAMP thread with attributes defined in the module.
  * @retval None
  */
void OpenampTask_Init(void);

/**
  * @brief Deinitialize the OpenAMP task and release resources.
  *        Terminates the OpenAMP thread and deinitializes OpenAMP.
  * @retval None
  */
void OpenampTask_DeInit(void);

/**
  * @brief  Post a command to the OpenAMP task queue.
  * @param  cmd: Command to post
  * @retval true if posted successfully, false otherwise
  */
bool OpenampTask_PostCommand(const OpenampTaskCommand_t *cmd);

#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_TASK_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

