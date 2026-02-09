/**
  ******************************************************************************
  * @file    app_common.h
  * @brief   Common macros and utilities for all tasks.
  ******************************************************************************
  */

#ifndef APP_COMMON_H
#define APP_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "logger_task.h"

#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_ERR     "\033[31m"
#define ANSI_COLOR_WRN     "\033[33m"
#define ANSI_COLOR_INF     "\033[32m"
#define ANSI_COLOR_DBG     "\033[36m"

#if REALTIME_DEBUG_LOG_ENABLED
#define APP_LOG(color, level, tag, fmt, ...) \
    do { \
        printf(color "[NS] [%s] [%s] " fmt ANSI_COLOR_RESET "\r\n", level, tag, ##__VA_ARGS__); \
    } while (0)

#define APP_LOG_ERR(tag, fmt, ...)  APP_LOG(ANSI_COLOR_ERR, "ERR", tag, fmt, ##__VA_ARGS__)
#define APP_LOG_WARN(tag, fmt, ...) APP_LOG(ANSI_COLOR_WRN, "WRN", tag, fmt, ##__VA_ARGS__)
#define APP_LOG_INF(tag, fmt, ...)  APP_LOG(ANSI_COLOR_INF, "INF", tag, fmt, ##__VA_ARGS__)
#define APP_LOG_DBG(tag, fmt, ...)  APP_LOG(ANSI_COLOR_DBG, "DBG", tag, fmt, ##__VA_ARGS__)

#else

#define APP_LOG_INF(tag, fmt, ...)      do { \
    LogMessage_t logMsg = { \
        .level = LOG_LEVEL_INFO, \
        .verbosity = LOG_VERBOSITY_LOW, \
        .taskName = tag, \
    }; \
    snprintf(logMsg.message, LOGGER_MSG_MAX_LEN, fmt, ##__VA_ARGS__); \
    LoggerTask_PostMessage(&logMsg); \
} while(0)

#define APP_LOG_ERR(tag, fmt, ...)      do { \
    LogMessage_t logMsg = { \
        .level = LOG_LEVEL_ERROR, \
        .verbosity = LOG_VERBOSITY_LOW, \
        .taskName = tag, \
    }; \
    snprintf(logMsg.message, LOGGER_MSG_MAX_LEN, fmt, ##__VA_ARGS__); \
    LoggerTask_PostMessage(&logMsg); \
} while(0)

#define APP_LOG_WARN(tag, fmt, ...)     do { \
    LogMessage_t logMsg = { \
        .level = LOG_LEVEL_WARN, \
        .verbosity = LOG_VERBOSITY_LOW, \
        .taskName = tag, \
    }; \
    snprintf(logMsg.message, LOGGER_MSG_MAX_LEN, fmt, ##__VA_ARGS__); \
    LoggerTask_PostMessage(&logMsg); \
} while(0)

#define APP_LOG_DBG(tag, fmt, ...) do { \
    LogMessage_t logMsg = { \
        .level = LOG_LEVEL_DEBUG, \
        .verbosity = LOG_VERBOSITY_MEDIUM, \
        .taskName = tag, \
    }; \
    snprintf(logMsg.message, LOGGER_MSG_MAX_LEN, fmt, ##__VA_ARGS__); \
    LoggerTask_PostMessage(&logMsg); \
} while(0)

#endif

/**
  * @brief  Handles application-level errors.
  */
void App_ErrorHandler(void);

#ifdef __cplusplus
}
#endif
#endif /* APP_COMMON_H */
