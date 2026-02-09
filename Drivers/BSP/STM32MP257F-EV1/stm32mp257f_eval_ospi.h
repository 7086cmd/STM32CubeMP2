/**
  ******************************************************************************
  * @file    stm32mp257f_eval_ospi.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32mp257f_eval_ospi.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32MP257F_EVAL_OSPI_H
#define __STM32MP257F_EVAL_OSPI_H

#ifdef __cplusplus
 extern "C" {
#endif

#define OSPI_BULK_ERASE_MAX_TIME 600000U
#define OSPI_DUMMY_CYCLES_CR_CFG 6U

/* Includes ------------------------------------------------------------------*/
#include "stm32mp2xx_hal.h"
#include "stm32mp257f_eval.h"

#include "mx25l512.h"

 /* Exported Variables --------------------------------------------------------*/
extern OSPI_HandleTypeDef hospi2;

 /** @addtogroup BSP
   * @{
   */
/* Exported Functions --------------------------------------------------------*/
 int32_t BSP_OSPI_Init(void);
 int32_t BSP_OSPI_DeInit(void);
 int32_t BSP_OSPI_GetStatus(void);
 int32_t BSP_OSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress);
 int32_t BSP_OSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size);
 int32_t BSP_OSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
 int32_t BSP_OSPI_Erase_Chip (void);

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __STM32MP257F_EVAL_OSPI_H */
