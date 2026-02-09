/**
  ******************************************************************************
  * @file    stm32mp257f_eval_sd.c
  * @author  MCD Application Team
  * @brief   This file includes driver for uSD card mounted on STM32MP257F-EV1
  *          board. Provides basic block read/write/erase operations and card
  *          state/info.
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
#include "stm32mp257f_eval_sd.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32MP257F_EVAL
  * @{
  */

/** @defgroup STM32MP257F_EVAL_SD
  * @{
  */

/* Private variables ---------------------------------------------------------*/
SD_HandleTypeDef hsd1; /* Single SD instance */

/* Private function prototypes -----------------------------------------------*/
static void SD_MspInit(SD_HandleTypeDef *hsd);
static void SD_MspDeInit(SD_HandleTypeDef *hsd);
__weak HAL_StatusTypeDef MX_SDMMC1_SD_Init(SD_HandleTypeDef *hsd);
/* Private functions ---------------------------------------------------------*/


extern void Error_Handler(void);

/**
  * @brief  Initializes the SD card (Instance 0 only).
  * @param  Instance SD Instance (only 0 is valid)
  * @retval BSP status
  */
int32_t BSP_SD_Init(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  /* Configure and enable Card Detect pin */

  /* Acquire GPIOD pins PD9 (SD Detect Pin) */
  if (ResMgr_GPIO_Request(RESMGR_RESOURCE_RIF_GPIOD, SD_DETECT_PIN) != RESMGR_STATUS_ACCESS_OK)
  {
    Error_Handler();
  }

#if !defined(A35_STARTUP_IN_ARM_MODE)
  /* Acquire RCC resource for GPIOD bank clock  */
  if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(93)))
  {
    SD_DETECT_GPIO_CLK_ENABLE();
  }

  if(IS_DEVELOPER_BOOT_MODE())
  {
    /* Enable power supplies required for detect pin */
    HAL_PWREx_EnableSupplyMonitoring(PWR_PVM_VDDIO3);
    HAL_PWREx_EnableSupply(PWR_PVM_VDDIO3);
    while (HAL_PWREx_PVM_IsBelowThreshold(PWR_PVM_VDDIO3)) { }
  }

#endif /*!A35_STARTUP_IN_ARM_MODE*/

  GPIO_InitTypeDef  gpio_init_structure;
  gpio_init_structure.Pin   = SD_DETECT_PIN;
  gpio_init_structure.Mode  = GPIO_MODE_INPUT;
  gpio_init_structure.Pull  = GPIO_PULLUP; /* Active low detect */
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_DETECT_GPIO_PORT, &gpio_init_structure);

  /* check card presence early */
  if (BSP_SD_IsDetected(Instance) != (int32_t)SD_PRESENT)
  {
    return BSP_ERROR_UNKNOWN_COMPONENT; /* No card inserted */
  }

  hsd1.Instance = SDMMC1;

  /* Low-level hardware init */
  SD_MspInit(&hsd1);

  /* HAL SD De-initialisation */
  if (HAL_SD_DeInit(&hsd1) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  /* Low-level hardware init */
  SD_MspInit(&hsd1);

  /* HAL SD initialization */
  if (MX_SDMMC1_SD_Init(&hsd1) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  return ret;
}

/**
  * @brief  DeInitializes the SD card.
  * @param  Instance SD Instance
  * @retval BSP status
  */
int32_t BSP_SD_DeInit(uint32_t Instance)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  int32_t ret = BSP_ERROR_NONE;

  if (HAL_SD_DeInit(&hsd1) != HAL_OK)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }

  SD_MspDeInit(&hsd1);

  /* DeInit detect pin */
  HAL_GPIO_DeInit(SD_DETECT_GPIO_PORT, SD_DETECT_PIN);

#if !defined(A35_STARTUP_IN_ARM_MODE)
  SD_DETECT_GPIO_CLK_DISABLE();
#endif /*!A35_STARTUP_IN_ARM_MODE*/

  return ret;
}

/**
  * @brief  Reads block(s) from the SD card in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to data buffer (32-bit aligned recommended)
  * @param  BlockIdx   Block (LBA) index
  * @param  BlocksNbr  Number of blocks to read
  * @retval BSP status
  */
int32_t BSP_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  uint32_t timeout = SD_READ_TIMEOUT * BlocksNbr;

  if (HAL_SD_ReadBlocks(&hsd1, (uint8_t*)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Writes block(s) to the SD card in polling mode.
  * @param  Instance   SD Instance
  * @param  pData      Pointer to data buffer
  * @param  BlockIdx   Block (LBA) index
  * @param  BlocksNbr  Number of blocks to write
  * @retval BSP status
  */
int32_t BSP_SD_WriteBlocks(uint32_t Instance, const uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  uint32_t timeout = SD_WRITE_TIMEOUT * BlocksNbr;

  if (HAL_SD_WriteBlocks(&hsd1, (uint8_t*)pData, BlockIdx, BlocksNbr, timeout) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Erases a range of blocks in the SD card.
  * @param  Instance    SD Instance
  * @param  StartBlock  First block index
  * @param  BlocksNbr   Number of blocks to erase
  * @retval BSP status
  */
int32_t BSP_SD_Erase(uint32_t Instance, uint32_t StartBlock, uint32_t BlocksNbr)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }

  uint32_t EndBlock = StartBlock + BlocksNbr; /* HAL expects end-exclusive */
  if (HAL_SD_Erase(&hsd1, StartBlock, EndBlock) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Get SD card information.
  * @param  Instance  SD Instance
  * @param  CardInfo  Pointer to info structure
  * @retval BSP status
  */
int32_t BSP_SD_GetCardInfo(uint32_t Instance, BSP_SD_CardInfo *CardInfo)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  if (HAL_SD_GetCardInfo(&hsd1, CardInfo) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }
  return BSP_ERROR_NONE;
}

/**
  * @brief  Get current card transfer state.
  * @param  Instance SD Instance
  * @retval SD_TRANSFER_OK or SD_TRANSFER_BUSY
  */
int32_t BSP_SD_GetCardState(uint32_t Instance)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM; /* Or define a dedicated error */
  }
  return (HAL_SD_GetCardState(&hsd1) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY;
}

/**
  * @brief  Detect if an SD card is present.
  * @param  Instance SD Instance
  * @retval SD_PRESENT or SD_NOT_PRESENT (or error code on wrong param)
  */
int32_t BSP_SD_IsDetected(uint32_t Instance)
{
  if (Instance >= SD_INSTANCES_NBR)
  {
    return BSP_ERROR_WRONG_PARAM;
  }
  /* Active low: line pulled low when card inserted */
  return (HAL_GPIO_ReadPin(SD_DETECT_GPIO_PORT, SD_DETECT_PIN) == GPIO_PIN_RESET) ? (int32_t)SD_PRESENT : (int32_t)SD_NOT_PRESENT;
}

/**
  * @brief  MX layer initialization helper (weak so user can override).
  * @param  hsd SD handle
  * @retval HAL status
  */
__weak HAL_StatusTypeDef MX_SDMMC1_SD_Init(SD_HandleTypeDef *hsd)
{
  HAL_StatusTypeDef ret = HAL_OK;

  /* HAL SD handle basic configuration */
  hsd->Instance                  = SDMMC1;
  hsd->Init.ClockEdge            = SDMMC_CLOCK_EDGE_RISING;
  hsd->Init.ClockPowerSave       = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd->Init.BusWide              = SDMMC_BUS_WIDE_1B;
  hsd->Init.HardwareFlowControl  = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd->Init.ClockDiv             = SDMMC_NSPEED_CLK_DIV;

  /* HAL SD initialization */
  if (HAL_SD_Init(hsd) != HAL_OK)
  {
    ret = HAL_ERROR;
  }
  else
  {
    /* Card info can be retrieved by user if needed */
    ret = HAL_OK;
  }

  return ret;
}

/* MSP Init/DeInit -----------------------------------------------------------*/
static void SD_MspInit(SD_HandleTypeDef *hsd)
{
  if (hsd->Instance == SDMMC1)
  {
    /* Acquire SDMMC1 using Resource manager */
    if (RESMGR_STATUS_ACCESS_OK != ResMgr_Request(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_SDMMC1_ID))
    {
      Error_Handler();
    }

    /* Acquire GPIOE pins PE0..PE5 (pins used by SDMMC1) */
    if (ResMgr_GPIO_Request(RESMGR_RESOURCE_RIF_GPIOE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 |
                            GPIO_PIN_0 | GPIO_PIN_1) != RESMGR_STATUS_ACCESS_OK)
    {
      Error_Handler();
    }

#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Acquire RCC resource for GPIOE bank clock  */
    if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(94)))
    {
      __HAL_RCC_GPIOE_CLK_ENABLE();
    }

    /* Enable power supplies required for SDMMC1 pins */
    HAL_PWREx_EnableSupplyMonitoring(PWR_PVM_VDDIO1);
    HAL_PWREx_EnableSupply(PWR_PVM_VDDIO1);
    while (HAL_PWREx_PVM_IsBelowThreshold(PWR_PVM_VDDIO1)) { }

#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* Configure PE pins: CK(PE3), CMD(PE2), D0(PE4), D1(PE5), D2(PE0), D3(PE1) */
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDMMC1;
    GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_0 | GPIO_PIN_1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    __HAL_RCC_SDMMC1_CLK_ENABLE();
    /* Force the SDMMC Periheral Clock Reset */
    __HAL_RCC_SDMMC1_FORCE_RESET();
    /* Release the SDMMC Periheral Clock Reset */
    __HAL_RCC_SDMMC1_RELEASE_RESET();

    /* Interrupt configuration */
#if defined (CORE_CM33)
    HAL_NVIC_SetPriority(SDMMC1_IRQn, BSP_SD_IT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
#endif
#if defined (CORE_CA35)
    IRQ_SetPriority(SDMMC1_IRQn, 0x01);
    IRQ_Enable(SDMMC1_IRQn);
#endif
  }
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd  SD handle
  * @retval None
  */
static void SD_MspDeInit(SD_HandleTypeDef *hsd)
{

  if (hsd->Instance == SDMMC1)
  {
    /* Disable interrupts */
  #if defined (CORE_CM33)
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
  #endif
  #if defined (CORE_CA35)
    IRQ_Disable(SDMMC1_IRQn);
  #endif

    /* DeInit GPIOE pins */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_0 | GPIO_PIN_1);

    /* Disable SDMMC1 clock */
    __HAL_RCC_SDMMC1_CLK_DISABLE();

    /* Release GPIO pins reserved for SDMMC1 */
    (void)ResMgr_GPIO_Release(RESMGR_RESOURCE_RIF_GPIOE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5 |
                            GPIO_PIN_0 | GPIO_PIN_1);

    (void)ResMgr_GPIO_Release(RESMGR_RESOURCE_RIF_GPIOD, SD_DETECT_PIN);


#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Release RCC resource used to enable GPIOE clock */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(94));
#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* Release SDMMC2 RIFSC reservation */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_SDMMC1_ID);
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
