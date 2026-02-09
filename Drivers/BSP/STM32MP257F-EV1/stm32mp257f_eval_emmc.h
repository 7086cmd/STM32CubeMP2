/**
  ******************************************************************************
  * @file    stm32mp257f_eval_emmc.h
  * @author  MCD Application Team
  * @brief   Header for eMMC BSP driver (SDMMC2) for STM32MP257F-EV1 board.
  *          Provides  read/write block access and boot partition configuration.
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

#ifndef STM32MP257F_EVAL_EMMC_H
#define STM32MP257F_EVAL_EMMC_H

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

/** @defgroup STM32MP257F_EVAL_EMMC eMMC
  * @brief eMMC BSP driver
  * @{ 
  */

/* Exported Types ------------------------------------------------------------*/
#define BSP_EMMC_CardInfo HAL_MMC_CardInfoTypeDef

/* Exported Constants --------------------------------------------------------*/
#define EMMC_INSTANCES_NBR              1UL

/* Timeouts (per block; multiplied by number of blocks) */
#ifndef EMMC_READ_TIMEOUT
#define EMMC_READ_TIMEOUT               100U
#endif
#ifndef EMMC_WRITE_TIMEOUT
#define EMMC_WRITE_TIMEOUT              500U
#endif

/* Transfer state */
#define EMMC_TRANSFER_OK                0U
#define EMMC_TRANSFER_BUSY              1U

/* eMMC is soldered: always detected */
#define EMMC_PRESENT                    1UL

/* Boot partition configuration field (EXT_CSD[179]) helpers */
#define EMMC_BOOT_PARTITION_ENABLE_POS  3U
#define EMMC_BOOT_ACK_POS               6U

/* Boot partition enable values (bits 5:3) */
#define EMMC_BOOT_PARTITION_USER        (0x07U << EMMC_BOOT_PARTITION_ENABLE_POS) /* Boot from user area */
#define EMMC_BOOT_PARTITION_BOOT1       (0x01U << EMMC_BOOT_PARTITION_ENABLE_POS)
#define EMMC_BOOT_PARTITION_BOOT2       (0x02U << EMMC_BOOT_PARTITION_ENABLE_POS)

/* Boot ACK bit (bit 6) */
#define EMMC_BOOT_ACK_ENABLE            (1U << EMMC_BOOT_ACK_POS)

/* (Optional) RW access bits (2:0) */
#define EMMC_BOOT_PARTITION_ACCESS_BOOT1_RW   (0x01U)
#define EMMC_BOOT_PARTITION_ACCESS_BOOT2_RW   (0x02U)

/* EXT_CSD byte index for partition config */
#define EMMC_EXT_CSD_PARTITION_CONFIG_INDEX   179U

/* Size of the EXT_CSD register (bytes) */
#define MMC_EXT_CSD_SIZE                       512U

/* Exported Variables --------------------------------------------------------*/
extern MMC_HandleTypeDef hmmc2;

/* Exported Functions --------------------------------------------------------*/
int32_t BSP_EMMC_Init(uint32_t Instance);
int32_t BSP_EMMC_DeInit(uint32_t Instance);
int32_t BSP_EMMC_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_EMMC_WriteBlocks(uint32_t Instance, const uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_EMMC_Erase(uint32_t Instance, uint32_t StartBlock, uint32_t BlocksNbr);
int32_t BSP_EMMC_GetCardInfo(uint32_t Instance, BSP_EMMC_CardInfo *CardInfo);
int32_t BSP_EMMC_GetCardState(uint32_t Instance);
int32_t BSP_EMMC_ConfigBootPartition(uint32_t Instance, uint32_t BootCfgBits);
int32_t BSP_EMMC_GetBootPartitionConfig(uint32_t Instance, uint32_t *pBootCfgBits);
int32_t BSP_EMMC_IsDetected(uint32_t Instance);

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

#endif /* STM32MP257F_EVAL_EMMC_H */
