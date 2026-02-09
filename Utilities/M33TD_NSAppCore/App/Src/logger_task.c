/**
  ******************************************************************************
  * @file    logger_task.c
  * @author  Application Team
  * @brief   Centralized logger task implementation file.
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
    * @addtogroup LoggerTask
    * @{
    */

/* Includes ------------------------------------------------------------------*/
#include "logger_task.h"
#include "logger_output_driver.h"
#include <stdio.h>
#include <string.h>

/* Private variables ---------------------------------------------------------*/
static osMessageQueueId_t LoggerQueueHandle = NULL;
static osThreadId_t LoggerTaskHandle = NULL;

static const osMessageQueueAttr_t LoggerQueueAttr = {
    .name = "LoggerQueue"
};

static const osThreadAttr_t LoggerTaskAttr = {
    .name = "LoggerTask",
    .priority = LOGGER_TASK_PRIORITY,
    .stack_size = LOGGER_TASK_STACK_SIZE
};

static LogVerbosity_t LoggerVerbosity = (LogVerbosity_t)LOGGER_VERBOSITY_DEFAULT;

/* Private function prototypes ------------------------------------------------*/
static void LoggerTask(void *argument);

/* Exported color variables (for use in output formatting) */
static char *LoggerLevelColors[LOG_LEVEL_COUNT] = {
    "\033[31m", // ERROR - Red
    "\033[33m", // WARN  - Yellow
    "\033[32m", // INFO  - Green
    "\033[36m"  // DEBUG - Cyan
};
static const char *LoggerColorReset = "\033[0m";

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Convert log level enum to string.
  * @param  level: Log level enum value.
  * @retval String representation of log level.
  */
static const char *LoggerLevelToString(LogLevel_t level)
{
    switch (level)
    {
        case LOG_LEVEL_ERROR:
            return "ERR";
        case LOG_LEVEL_WARN:
            return "WRN";
        case LOG_LEVEL_INFO:
            return "INF";
        case LOG_LEVEL_DEBUG:
            return "DBG";
        default:
            return "UNK";
    }
}

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Initialize the Logger Task and its resources.
  *         Creates the logger queue and the task thread.
  * @retval None
  */
void LoggerTask_Init(void)
{

    LoggerQueueHandle = osMessageQueueNew(LOGGER_QUEUE_SIZE, sizeof(LogMessage_t), &LoggerQueueAttr);
    if (LoggerQueueHandle == NULL)
    {
        NSAppCore_ErrorHandler();
    }
    LoggerTaskHandle = osThreadNew(LoggerTask, NULL, &LoggerTaskAttr);
    if (LoggerTaskHandle == NULL)
    {
        NSAppCore_ErrorHandler();
    }

    logger_output_init();
}

/**
  * @brief  De-initialize the Logger Task and release its resources.
  * @retval None
  */
void LoggerTask_DeInit(void)
{
    if (LoggerQueueHandle)
    {
        osMessageQueueDelete(LoggerQueueHandle);
        LoggerQueueHandle = NULL;
    }
    if (LoggerTaskHandle)
    {
        osThreadTerminate(LoggerTaskHandle);
        LoggerTaskHandle = NULL;
    }
}

/**
  * @brief  Post a message to the logger task.
  * @param  msg: Pointer to the log message structure to enqueue.
  * @retval true if the message was successfully enqueued, false otherwise.
  */
bool LoggerTask_PostMessage(const LogMessage_t *msg)
{
    if (!LoggerQueueHandle || !msg)
        return false;
    return osMessageQueuePut(LoggerQueueHandle, msg, 0, 0) == osOK;
}

/**
  * @brief  Logger task function.
  * @param  argument: Not used.
  * @retval None
  */
static void LoggerTask(void *argument)
{
    LogMessage_t msg;
    for (;;)
    {
        if (osMessageQueueGet(LoggerQueueHandle, &msg, NULL, osWaitForever) == osOK)
        {
            if (msg.verbosity <= LoggerVerbosity)
            {
                printf("%s[NS] [%s] [%s] %s%s\r\n",
                    LoggerLevelColors[msg.level],
                    LoggerLevelToString(msg.level),
                    msg.taskName ? msg.taskName : "SYS",
                    msg.message,
                    LoggerColorReset);
            }
        }
    }
}

/** @} */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

