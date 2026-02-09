/**
  ******************************************************************************
  * @file    openamp_driver.h
  * @author  MCD Application Team
  * @brief   Interface for OpenAMP platform driver abstraction.
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

#ifndef OPENAMP_DRIVER_H
#define OPENAMP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief  OpenAMP platform driver interface.
  *
  * Project provides one concrete driver by defining the global instance:
  * `OpenampDriverTypeDef openampDriver;`
  */
typedef struct
{
  int  (*init)(void);
  void (*deinit)(void);
} OpenampDriverTypeDef;

/**
  * @brief  Global instance of the OpenAMP platform driver.
  * @note   Project must define this symbol in exactly one C file.
  */
extern OpenampDriverTypeDef openampDriver;

#ifdef __cplusplus
}
#endif

#endif /* OPENAMP_DRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

