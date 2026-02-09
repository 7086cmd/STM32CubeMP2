/**
  ******************************************************************************
  * @file    led_driver.h
  * @author  MCD Application Team
  * @brief   Abstract interface for LED control.
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
  * @addtogroup LedDriver
  * @{
  */

#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief LED identifier enumeration.
  */
typedef enum
{
  LED_ID_1 = 0, /**< LED 1 identifier. */
  LED_ID_2,     /**< LED 2 identifier. */
  LED_ID_3,     /**< LED 3 identifier. */
  LED_ID_4,     /**< LED 4 identifier. */
  LED_ID_MAX    /**< Maximum number of LEDs. */
} LedId_t;

/**
  * @brief LED driver interface.
  *
  * Project provides one concrete driver by defining the global instance:
  * `LedDriverTypeDef led_driver;`
  */
typedef struct
{
  void (*init)(void);
  void (*deinit)(void);
  void (*led_toggle)(LedId_t led_id);
  void (*led_on)(LedId_t led_id);
  void (*led_off)(LedId_t led_id);
} LedDriverTypeDef;

/**
  * @brief Global LED driver instance.
  * @note  Project must define this symbol in exactly one C file.
  */
extern LedDriverTypeDef led_driver;

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

#endif /* LED_DRIVER_H */