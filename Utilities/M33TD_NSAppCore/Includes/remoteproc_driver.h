/**
  ******************************************************************************
  * @file    remoteproc_driver.h
  * @author  MCD Application Team
  * @brief   Remote processor driver interface.
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

#ifndef REMOTEPROC_DRIVER_H
#define REMOTEPROC_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
  * @brief  Callback type for remote processor crash events.
  */
typedef void (*RemoteProcCrashCallback)(uint32_t cpu_id);

/**
  * @brief Remote processor driver interface.
  *
  * Project provides one concrete driver by defining the global instance:
  * `RemoteProcDriverTypeDef remoteproc_driver;`
  */
typedef struct
{
  int (*init)(void);
  int (*deinit)(void);
  int (*register_crash_callback)(RemoteProcCrashCallback cb);
  int (*unregister_crash_callback)(RemoteProcCrashCallback cb);
} RemoteProcDriverTypeDef;

/**
  * @brief Global remote processor driver instance.
  * @note  Project must define this symbol in exactly one C file.
  */
extern RemoteProcDriverTypeDef remoteproc_driver;

#ifdef __cplusplus
}
#endif

#endif /* REMOTEPROC_DRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/