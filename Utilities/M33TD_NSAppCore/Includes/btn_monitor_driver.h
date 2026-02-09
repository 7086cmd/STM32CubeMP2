/**
  ******************************************************************************
  * @file    btn_monitor_driver.h
  * @author  MCD Application Team
  * @brief   Interface for Button Monitor driver abstraction.
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

#ifndef BTN_MONITOR_DRIVER_H
#define BTN_MONITOR_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/**
  * @brief  ISR-context callback invoked by the button driver on a button edge.
  * @note   The implementation must be ISR-safe.
  */
typedef void (*BtnMonitorDriver_IRQCallback)(void *context);

/**
  * @brief  Interface for button monitor driver operations.
  * 
  * Project provides one concrete driver by defining the global instance:
  */
typedef struct
{
  int (*init)(BtnMonitorDriver_IRQCallback cb, void *context);
  int (*deinit)(void);
  void (*enable_btn_irq)(void);
  void (*disable_btn_irq)(void);
  bool (*is_btn_pressed)(void);
} BtnMonitorDriverTypeDef;

/**
  * @brief  Global instance of the button monitor driver interface.
  * @note   Project must define this symbol in exactly one C file.
  */
extern BtnMonitorDriverTypeDef btnMonitorDriver;

#ifdef __cplusplus
}
#endif

#endif /* BTN_MONITOR_DRIVER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

