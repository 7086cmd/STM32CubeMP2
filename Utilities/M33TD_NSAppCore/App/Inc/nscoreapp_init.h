/**
  ******************************************************************************
  * @file    nscoreapp_init.h
  * @author  MCD Application Team
  * @brief   Non-secure core application initialization API.
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

#ifndef NSCOREAPP_INIT_H
#define NSCOREAPP_INIT_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "nsappcore_config.h"

/**
    * @brief  Initialize all non-secure core application tasks and modules.
    *         This includes display, logger, OpenAMP, SCMI manager, remoteproc, and watchdog monitor tasks.
    */
void NSCoreApp_Init(void);

/**
    * @brief  De-initialize all non-secure core application tasks and modules.
    *         This includes display, logger, OpenAMP, SCMI manager, remoteproc, and watchdog monitor tasks.
    */
void NSCoreApp_DeInit(void);

#ifdef __cplusplus
}
#endif

#endif /* NSCOREAPP_INIT_H */

