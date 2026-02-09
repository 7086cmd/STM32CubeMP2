/**
  ******************************************************************************
  * @file    stm32mp257f_eval_sd.h
  * @author  MCD Application Team
  * @brief   Header for SD card BSP driver for STM32MP257F-EV1 board.
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

#ifndef STM32MP257F_EVAL_SD_H
#define STM32MP257F_EVAL_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32mp2xx_hal.h"
#include "stm32mp257f_eval.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32MP257F_EVAL
  * @{
  */

/** @defgroup STM32MP257F_EVAL_SD SD
  * @brief SD BSP driver (polling transfers only)
  * @{ 
  */

/* Exported Types ------------------------------------------------------------*/
#define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef

/* Exported Constants --------------------------------------------------------*/
#define SD_INSTANCES_NBR            1UL

/* Timeouts (per block; multiplied by number of blocks) */
#ifndef SD_READ_TIMEOUT
#define SD_READ_TIMEOUT             100U
#endif
#ifndef SD_WRITE_TIMEOUT
#define SD_WRITE_TIMEOUT            500U
#endif

/* Transfer state */
#define SD_TRANSFER_OK              0U
#define SD_TRANSFER_BUSY            1U

/* Card detect */
#define SD_PRESENT                  1UL
#define SD_NOT_PRESENT              0UL

/* Card Detect pin (PD9, input pull-up, active low) */
#define SD_DETECT_PIN               GPIO_PIN_9
#define SD_DETECT_GPIO_PORT         GPIOD
#define SD_DETECT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define SD_DETECT_GPIO_CLK_DISABLE() __HAL_RCC_GPIOD_CLK_DISABLE()

/* Exported Variables --------------------------------------------------------*/
extern SD_HandleTypeDef hsd1;

/* Exported Functions --------------------------------------------------------*/
int32_t BSP_SD_Init(uint32_t Instance);
int32_t BSP_SD_DeInit(uint32_t Instance);
int32_t BSP_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_SD_WriteBlocks(uint32_t Instance, const uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_SD_Erase(uint32_t Instance, uint32_t StartBlock, uint32_t BlocksNbr);
int32_t BSP_SD_GetCardInfo(uint32_t Instance, BSP_SD_CardInfo *CardInfo);
int32_t BSP_SD_GetCardState(uint32_t Instance);
int32_t BSP_SD_IsDetected(uint32_t Instance);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32MP257F_EVAL_SD_H */
