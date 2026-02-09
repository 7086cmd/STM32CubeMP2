/**
  ******************************************************************************
  * @file    logger_task.h
  * @author  Application Team
  * @brief   Header file for centralized logger task implementation.
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

#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "nsappcore_config.h"

/* Exported constants -------------------------------------------------------- */
#define LOGGER_QUEUE_SIZE           10     /**< Queue size for Logger Task. */
#define LOGGER_MSG_MAX_LEN          128    /**< Max length for log message buffer. */

/* Exported types ------------------------------------------------------------ */
/**
  * @brief  Enumeration for log levels.
  */
typedef enum {
    LOG_LEVEL_ERROR = 0, /*!< Error log */
    LOG_LEVEL_WARN,      /*!< Warning log */
    LOG_LEVEL_INFO,      /*!< Information log */
    LOG_LEVEL_DEBUG,     /*!< Debug log */
    LOG_LEVEL_COUNT      /*!< Number of log levels */
} LogLevel_t;

/**
  * @brief  Enumeration for log verbosity.
  */
typedef enum {
    LOG_VERBOSITY_LOW = 0,    /*!< Low verbosity */
    LOG_VERBOSITY_MEDIUM,     /*!< Medium verbosity */
    LOG_VERBOSITY_HIGH        /*!< High verbosity */
} LogVerbosity_t;

/**
  * @brief  Structure for logger messages.
  */
typedef struct {
    LogLevel_t level;              /*!< Log level */
    LogVerbosity_t verbosity;      /*!< Log verbosity */
    const char *taskName;          /*!< Source task name */
    char message[LOGGER_MSG_MAX_LEN]; /*!< Log message buffer */
} LogMessage_t;

/* Exported variables -------------------------------------------------------- */

/* Exported functions prototypes --------------------------------------------- */
/**
  * @brief  Initialize the Logger Task and its resources.
  *         Creates the logger queue and the task thread.
  * @retval None
  */
void LoggerTask_Init(void);

/**
  * @brief  De-initialize the Logger Task and release its resources.
  * @retval None
  */
void LoggerTask_DeInit(void);

/**
  * @brief  Post a message to the logger task.
  * @param  msg: Pointer to the log message structure to enqueue.
  * @retval true if the message was successfully enqueued, false otherwise.
  */
bool LoggerTask_PostMessage(const LogMessage_t *msg);

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_TASK_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/