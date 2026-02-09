/**
  ******************************************************************************
  * @file    stm32mp257f_eval_emmc.c
  * @author  MCD Application Team
  * @brief   eMMC BSP driver for STM32MP257F-EV1 board (SDMMC2, 8-bit bus).
  *          Features: Init/DeInit, block Read/Write, Erase, Card Info/State,
  *          Boot Partition configuration (EXT_CSD[179]) using minimal
  *          SWITCH command sequence.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32mp257f_eval_emmc.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32MP257F_EVAL
  * @{
  */

/** @defgroup STM32MP257F_EVAL_EMMC
  * @{
  */

/* Private variables ---------------------------------------------------------*/
MMC_HandleTypeDef hmmc2; /* Single eMMC instance */

/* Private function prototypes -----------------------------------------------*/
static void EMMC_MspInit(MMC_HandleTypeDef *hmmc);
static void EMMC_MspDeInit(MMC_HandleTypeDef *hmmc);
__weak HAL_StatusTypeDef MX_SDMMC2_MMC_Init(MMC_HandleTypeDef *hmmc);
/* Private functions ---------------------------------------------------------*/

/* Local helpers for SWITCH command */
static uint32_t EMMC_SendStatus(MMC_HandleTypeDef *hmmc, uint32_t *pResp);
static uint32_t EMMC_SwitchBootPartition(MMC_HandleTypeDef *hmmc, uint32_t BootCfgBits);

extern void Error_Handler(void);

/**
  * @brief  Initialize eMMC (Instance 0 only).
  * @param  Instance eMMC instance (only 0 valid)
  * @retval BSP status
  */
int32_t BSP_EMMC_Init(uint32_t Instance)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  hmmc2.Instance = SDMMC2;

  /* Low-level MSP Init */
  EMMC_MspInit(&hmmc2);

  /* HAL MMC init */
  if (MX_SDMMC2_MMC_Init(&hmmc2) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  return BSP_ERROR_NONE;
}

/**
  * @brief  DeInitialize eMMC.
  * @param  Instance eMMC instance
  * @retval BSP status
  */
int32_t BSP_EMMC_DeInit(uint32_t Instance)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  int32_t ret = BSP_ERROR_NONE;
  if (HAL_MMC_DeInit(&hmmc2) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  EMMC_MspDeInit(&hmmc2);

  return ret;
}

/**
  * @brief  Read blocks (polling).
  * @param  Instance  eMMC instance
  * @param  pData     Buffer
  * @param  BlockIdx  Start block (LBA)
  * @param  BlocksNbr Number of blocks
  * @retval BSP status
  */
int32_t BSP_EMMC_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  uint32_t timeout = EMMC_READ_TIMEOUT * BlocksNbr;
  if (HAL_MMC_ReadBlocks(&hmmc2, (uint8_t*)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Write blocks (polling).
  */
int32_t BSP_EMMC_WriteBlocks(uint32_t Instance, const uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  uint32_t timeout = EMMC_WRITE_TIMEOUT * BlocksNbr;
  if (HAL_MMC_WriteBlocks(&hmmc2, (uint8_t*)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Erase blocks.
  * @param  StartBlock First block
  * @param  BlocksNbr  Number of blocks
  */
int32_t BSP_EMMC_Erase(uint32_t Instance, uint32_t StartBlock, uint32_t BlocksNbr)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  uint32_t EndBlock = StartBlock + BlocksNbr; /* end-exclusive */
  if (HAL_MMC_Erase(&hmmc2, StartBlock, EndBlock) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Get card info.
  */
int32_t BSP_EMMC_GetCardInfo(uint32_t Instance, BSP_EMMC_CardInfo *CardInfo)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  if (HAL_MMC_GetCardInfo(&hmmc2, CardInfo) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Get transfer state.
  */
int32_t BSP_EMMC_GetCardState(uint32_t Instance)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  return (HAL_MMC_GetCardState(&hmmc2) == HAL_MMC_CARD_TRANSFER) ? (int32_t)EMMC_TRANSFER_OK : (int32_t)EMMC_TRANSFER_BUSY;
}

/**
  * @brief  Configure eMMC boot partition (EXT_CSD[179]).
  * @param  BootCfgBits Combination of EMMC_BOOT_PARTITION_* and EMMC_BOOT_ACK_ENABLE and optional access bits.
  * @note   Performs a SWITCH command (CMD6) to write EXT_CSD byte 179. Waits for completion via status polling.
  */
int32_t BSP_EMMC_ConfigBootPartition(uint32_t Instance, uint32_t BootCfgBits)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  /* Minimal validation: only bits 6:0 allowed */
  if ((BootCfgBits & ~0x7FU) != 0U)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  if (EMMC_SwitchBootPartition(&hmmc2, BootCfgBits) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Retrieve current boot partition configuration (EXT_CSD[179]).
  * @param  pBootCfgBits Pointer to store byte value.
  */
int32_t BSP_EMMC_GetBootPartitionConfig(uint32_t Instance, uint32_t *pBootCfgBits)
{
  if ((Instance >= EMMC_INSTANCES_NBR) || (pBootCfgBits == NULL))
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  uint32_t ext_csd[MMC_EXT_CSD_SIZE/4];
  if (HAL_MMC_GetCardExtCSD(&hmmc2, ext_csd, HAL_MAX_DELAY) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  uint8_t *ext_csd_bytes = (uint8_t*)ext_csd;
  *pBootCfgBits = ext_csd_bytes[EMMC_EXT_CSD_PARTITION_CONFIG_INDEX];
  return BSP_ERROR_NONE;
}

/**
  * @brief  eMMC presence (always present on board).
  */
int32_t BSP_EMMC_IsDetected(uint32_t Instance)
{
  if (Instance >= EMMC_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  return (int32_t)EMMC_PRESENT;
}


static uint32_t EMMC_SendStatus(MMC_HandleTypeDef *hmmc, uint32_t *pResp)
{
  uint32_t errorstate;
  errorstate = SDMMC_CmdSendStatus(hmmc->Instance, (uint32_t)(hmmc->MmcCard.RelCardAdd << 16U));
  if (errorstate != HAL_MMC_ERROR_NONE)
  {
    return errorstate;
  }
  *pResp = SDMMC_GetResponse(hmmc->Instance, SDMMC_RESP1);
  return HAL_MMC_ERROR_NONE;
}

static uint32_t EMMC_SwitchBootPartition(MMC_HandleTypeDef *hmmc, uint32_t BootCfgBits)
{
  if (hmmc->State != HAL_MMC_STATE_READY)
  {
    return HAL_BUSY;
  }
  uint32_t resp = 0U;
  uint32_t errorstate = HAL_MMC_ERROR_NONE;
  uint32_t trials = SDMMC_MAX_TRIAL;

  hmmc->State = HAL_MMC_STATE_BUSY;

  /* CMD6 argument: [31:26] access=0x03 (write byte), [25:16] index=179, [15:8] value, [7:3] cmd set=0, [2:0] reserved */
  uint32_t argument = (0x03U << 24) | (EMMC_EXT_CSD_PARTITION_CONFIG_INDEX << 16) | (BootCfgBits << 8);
  errorstate = SDMMC_CmdSwitch(hmmc->Instance, argument);
  if (errorstate == HAL_MMC_ERROR_NONE)
  {
    /* Poll card status until ready for data or timeout */
    do
    {
      errorstate = EMMC_SendStatus(hmmc, &resp);
      if (errorstate != HAL_MMC_ERROR_NONE)
      {
        break;
      }
      trials--;
    } while (((resp & 0x100U) == 0U) && (trials != 0U)); /* Ready for data bit */

    if ((trials == 0U) && (errorstate == HAL_MMC_ERROR_NONE))
    {
      errorstate = SDMMC_ERROR_TIMEOUT;
    }
    else if ((resp & 0x80U) != 0U) /* SWITCH_ERROR bit */
    {
      errorstate = SDMMC_ERROR_GENERAL_UNKNOWN_ERR;
    }
  }

  hmmc->State = HAL_MMC_STATE_READY;

  if (errorstate != HAL_MMC_ERROR_NONE)
  {
    __HAL_MMC_CLEAR_FLAG(hmmc, SDMMC_STATIC_FLAGS);
    hmmc->ErrorCode |= errorstate;
    return HAL_ERROR;
  }
  return HAL_OK;
}

/**
  * @brief  Weak MX init (user overrideable).
  */
__weak HAL_StatusTypeDef MX_SDMMC2_MMC_Init(MMC_HandleTypeDef *hmmc)
{
  HAL_StatusTypeDef ret = HAL_OK;
  hmmc->Instance                 = SDMMC2;
  hmmc->Init.ClockEdge           = SDMMC_CLOCK_EDGE_RISING;
  hmmc->Init.ClockPowerSave      = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hmmc->Init.BusWide             = SDMMC_BUS_WIDE_8B;
  hmmc->Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hmmc->Init.ClockDiv            = 2;

  if (HAL_MMC_Init(hmmc) != HAL_OK)
  {
    ret = HAL_ERROR;
  }
  return ret;
}

/* MSP Init/DeInit -----------------------------------------------------------*/
static void EMMC_MspInit(MMC_HandleTypeDef *hmmc)
{
  if (hmmc->Instance == SDMMC2)
  {

    /* Acquire SDMMC2 using Resource manager */
    if (RESMGR_STATUS_ACCESS_OK != ResMgr_Request(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_SDMMC2_ID))
    {
      Error_Handler();
    }

    /* Acquire GPIOE pins PE6..PE15 (pins used by SDMMC2) */
    if (ResMgr_GPIO_Request(RESMGR_RESOURCE_RIF_GPIOE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                            GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
        != RESMGR_STATUS_ACCESS_OK)
    {
      Error_Handler();
    }

#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Acquire RCC resource for GPIOE bank clock  */
    if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(94)))
    {
      __HAL_RCC_GPIOE_CLK_ENABLE();
    }

    /* Enable supply for SDMMC2 IO bank (VDDIO2) */
    HAL_PWREx_EnableSupplyMonitoring(PWR_PVM_VDDIO2);
    HAL_PWREx_EnableSupply(PWR_PVM_VDDIO2);
    while(HAL_PWREx_PVM_IsBelowThreshold(PWR_PVM_VDDIO2));

#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* PE6..PE15 AF12 SDMMC2: D0-D7, CMD, CLK  */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_SDMMC2;
    GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                                GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    __HAL_RCC_SDMMC2_CLK_ENABLE();
    /* Force the SDMMC Periheral Clock Reset */
    __HAL_RCC_SDMMC2_FORCE_RESET();
    /* Release the SDMMC Periheral Clock Reset */
    __HAL_RCC_SDMMC2_RELEASE_RESET();
  }
}

static void EMMC_MspDeInit(MMC_HandleTypeDef *hmmc)
{
  if (hmmc->Instance == SDMMC2)
  {
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /* Release GPIOE pins reserved for SDMMC2 */
    (void)ResMgr_GPIO_Release(RESMGR_RESOURCE_RIF_GPIOE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 |
                             GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Release RCC resource used to enable GPIOE clock */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(94));
#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* Release SDMMC2 RIFSC reservation */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_SDMMC2_ID);
  }

}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
