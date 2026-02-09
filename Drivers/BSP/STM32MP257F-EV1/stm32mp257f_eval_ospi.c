/**
  ******************************************************************************
  * @file    stm32mp257f_eval_ospi.c
  * @author  MCD Application Team
  * @brief   This file includes a standard driver for the MX25L512 QSPI
  *          memory mounted on STM32MP57F_EVAL board.
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


  /* Includes ------------------------------------------------------------------*/
#include "stm32mp257f_eval_ospi.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32MP257F_EVAL
  * @{
  */

/** @defgroup STM32MP257F_EVAL_OSPI
  * @{
  */

/* Private variables ---------------------------------------------------------*/
OSPI_HandleTypeDef hospi2; /* Single sNOR instance */

/* Private function prototypes -----------------------------------------------*/
static void OSPI_MspInit(OSPI_HandleTypeDef *hospi);
static void OSPI_MspDeInit(OSPI_HandleTypeDef *hospi);
static uint8_t OSPI_ResetChip(void);
static uint8_t OSPI_WriteEnable(void);
static uint8_t OSPI_AutoPollingMemReady(uint32_t);
static uint8_t OSPI_ReadDeviceID(void);
static uint8_t OSPI_SetQpiMode(void);

__weak HAL_StatusTypeDef MX_OSPI_Init(OSPI_HandleTypeDef *hospi);


extern void Error_Handler(void);


/* USER CODE BEGIN 1 */

/* BSP OSPI init function */
int32_t BSP_OSPI_Init(void)
{
  /* prepare XSPI peripheral for ST-Link Utility operations */
  /* Low-level MSP Init */
  hospi2.Instance = OCTOSPI2;

  OSPI_MspInit(&hospi2);

  if (HAL_OSPI_DeInit(&hospi2) != HAL_OK)
  {
     return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Low-level MSP Init */
  OSPI_MspInit(&hospi2);

  if (MX_OSPI_Init(&hospi2) != 0)
  {
     return BSP_ERROR_PERIPH_FAILURE;
  }

  /* OSPI memory reset */
  if (OSPI_ResetChip() != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  HAL_Delay(1);

  /* Enable QPI mode */
  if (OSPI_SetQpiMode() != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  if (OSPI_ReadDeviceID() != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Wait for the OSPI memory to become ready */
  uint32_t start = HAL_GetTick();
  while ((HAL_GetTick() - start) < HAL_OSPI_TIMEOUT_DEFAULT_VALUE)
  {
    if (BSP_OSPI_GetStatus() == BSP_ERROR_NONE)
    {
      break;
    }
  }

  return BSP_ERROR_NONE;
}

/* BSP OSPI Deinit function */
int32_t BSP_OSPI_DeInit(void)
{
  if (HAL_OSPI_DeInit(&hospi2) != HAL_OK)
  {
     return BSP_ERROR_PERIPH_FAILURE;
  }

  OSPI_MspDeInit(&hospi2);

  return BSP_ERROR_NONE;
}


/**
* @brief Get the status of the OSPI instance
* @param None
* @retval 0 if the OSPI is ready 1 otherwise
*/
int32_t BSP_OSPI_GetStatus(void)
{
  /* USER CODE BEGIN POST_OSPI_GET_STATUS */
  uint8_t status = BSP_ERROR_NONE;
  OSPI_RegularCmdTypeDef s_command;
  uint8_t reg[2];

  /* Initialize the read status register command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_READ_STATUS_REG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 1;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Reception of the data */
  if (HAL_OSPI_Receive(&hospi2, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check the value of the register */
  if ((reg[0] & MX25L512_SR_WIP) != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* USER CODE END POST_OSPI_GET_STATUS */

  return status;
}

int32_t BSP_OSPI_Erase_Chip(void)
{

  uint8_t status= BSP_ERROR_NONE;

  OSPI_RegularCmdTypeDef s_command;

  /* Initialize the erase command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 0;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.Instruction           = MX25L512_BULK_ERASE_CMD;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;

  /* Enable write operations */
  if (OSPI_WriteEnable() != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Send the command */
  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (OSPI_AutoPollingMemReady(OSPI_BULK_ERASE_MAX_TIME) != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  return status;
}

int32_t BSP_OSPI_EraseSector(uint32_t EraseStartAddress, uint32_t EraseEndAddress)
{
  uint8_t status = BSP_ERROR_NONE;

  OSPI_RegularCmdTypeDef s_command;

  EraseStartAddress = EraseStartAddress - EraseStartAddress % MX25L512_SECTOR_SIZE;

  /* Initialize the erase command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 0;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.Instruction           = MX25L512_SECTOR_ERASE_4K_CMD;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;

  while (EraseEndAddress >= EraseStartAddress)
  {
    s_command.Address = (EraseStartAddress & 0x0FFFFFFF);

    /* Enable write operations */
    if (OSPI_WriteEnable() != 0)
    {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    /* Send the command */
    if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
    {
      return BSP_ERROR_PERIPH_FAILURE;
    }

    /* Configure automatic polling mode to wait for end of erase */
    if (OSPI_AutoPollingMemReady(OSPI_BULK_ERASE_MAX_TIME) != 0)
    {
        return BSP_ERROR_PERIPH_FAILURE;
    }

    EraseStartAddress += MX25L512_SECTOR_SIZE;
  }

  return status;
}

/**
  * @brief  Writes data into the OSPI memory.
  * @param  buffer: the source buffer
  * @param  address: the start address to write into
  * @param  buffer_size: Size of data to write
  * @retval 0 on Success 1 on Failure
  */
int32_t BSP_OSPI_WriteMemory(uint8_t* buffer, uint32_t address, uint32_t buffer_size)
{

  OSPI_RegularCmdTypeDef s_command = {0};
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_addr = 0;

  while (current_addr <= address) {
      current_addr += MX25L512_PAGE_SIZE;
  }
  current_size = current_addr - address;

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > buffer_size) {
      current_size = buffer_size;
  }

  /* Initialize the adress variables */
  current_addr = address;
  end_addr = address + buffer_size;

  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.Instruction           = MX25L512_PAGE_PROG_CMD;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.DummyCycles           = 0;

  /* Perform the write page by page */
  do {
      s_command.Address = current_addr;
      s_command.NbData  = current_size;

      if (current_size == 0)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }

      /* Enable write operations */
      if (OSPI_WriteEnable() != 0)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }

      /* Configure the command */
      if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }

      /* Transmission of the data */

      if (HAL_OSPI_Transmit(&hospi2, (uint8_t*)buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }


      /* Configure automatic polling mode to wait for end of program */
      if (OSPI_AutoPollingMemReady(HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
      {
        return BSP_ERROR_PERIPH_FAILURE;
      }

      /* Update the address and size variables for next page programming */
      current_addr += current_size;
      buffer += current_size;
      current_size = ((current_addr + MX25L512_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25L512_PAGE_SIZE;
  } while (current_addr < end_addr);

  return BSP_ERROR_NONE;
}

/**
  * @brief  Reads an amount of data from the XSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read
  * @retval 0 on Success 1 on Failure
  */
int32_t BSP_OSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
  OSPI_RegularCmdTypeDef s_command = {0};

  /* Initialize the read command */
  s_command.Instruction           = MX25L512_4IO_FAST_READ_CMD;
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Address               = ReadAddr;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_4_LINES;
  s_command.AddressSize           = HAL_OSPI_ADDRESS_24_BITS;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.AlternateBytesDtrMode = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.NbData                = Size;
  s_command.DummyCycles           = OSPI_DUMMY_CYCLES_CR_CFG;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;

  /* Configure the command */
  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }


  /* Reception of the data */

  if (HAL_OSPI_Receive(&hospi2, (uint8_t*)pData, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != 0)
  {
    return BSP_ERROR_PERIPH_FAILURE;
  }

  /* Check for the reception of data */

  return BSP_ERROR_NONE;
}

static uint8_t OSPI_ResetChip(void)
{
  uint8_t status = 0;

  OSPI_RegularCmdTypeDef s_command;
  OSPI_AutoPollingTypeDef s_config;

  /* Initialize the reset enable command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_RESET_ENABLE_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Send the reset memory command */
  s_command.Instruction = MX25L512_RESET_MEMORY_CMD;
  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  s_command.Instruction  = MX25L512_READ_STATUS_REG_CMD;
  s_command.DataMode     = HAL_OSPI_DATA_1_LINE;
  s_command.NbData       = 1;
  s_command.DataDtrMode  = HAL_OSPI_DATA_DTR_DISABLE;

  s_config.Match         = 0;
  s_config.Mask          = MX25L512_SR_WIP;
  s_config.MatchMode     = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval      = 0x10;
  s_config.AutomaticStop = HAL_OSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  if (HAL_OSPI_AutoPolling(&hospi2, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* USER CODE BEGIN OSPI_MEMORY_RESET_CMD */

  /* USER CODE END OSPI_MEMORY_RESET_CMD */

  return status;
}

static uint8_t OSPI_WriteEnable(void)
{
  uint8_t status = 0;

  OSPI_RegularCmdTypeDef  s_command;
  OSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_WRITE_ENABLE_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;

  /* DTR mode is disabled */
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Initialize the read status register command */
  s_command.Instruction           = MX25L512_READ_STATUS_REG_CMD;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.NbData                = 2;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  /* Configure automatic polling mode to wait for WEL enabling */
  s_config.MatchMode               = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval                = 0x10;
  s_config.AutomaticStop           = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  s_config.Match                   = MX25L512_SR_WREN;
  s_config.Mask                    = MX25L512_SR_WREN;

  if (HAL_OSPI_AutoPolling(&hospi2, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

 /* USER CODE BEGIN OSPI_WRITE_ENABLE_CMD */

 /* USER CODE END OSPI_WRITE_ENABLE_CMD */

  return status;
}

static uint8_t OSPI_AutoPollingMemReady(uint32_t timeout)
{
  uint8_t status = 0;

  OSPI_RegularCmdTypeDef  s_command;
  OSPI_AutoPollingTypeDef s_config;
  uint8_t reg[2];
  uint32_t start = HAL_GetTick();

  /* Configure automatic polling mode to wait for memory ready */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_READ_STATUS_REG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.Address               = 0;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.NbData                = 2;
  s_command.DummyCycles           = 0x00;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressDtrMode        = HAL_OSPI_ADDRESS_DTR_DISABLE;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;

  s_config.Match                  = 0;
  s_config.Mask                   = MX25L512_SR_WIP;

  while( HAL_GetTick() - start < timeout)
  {
     if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      status = 1;
      break;
    }

    if (HAL_OSPI_Receive(&hospi2, reg, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      status = 1;
      break;
    }
    /* the Memory is ready, break from the loop */
    if ((reg[0] & s_config.Mask ) == s_config.Match)
    {
      break;
    }
  }
  /* USER CODE BEGIN OSPI_AUTO_POLLING_READY */

  /* USER CODE END OSPI_AUTO_POLLING_READY */
  return status;
}

static uint8_t OSPI_ReadDeviceID(void)
{
  uint8_t status = 0;
  OSPI_RegularCmdTypeDef s_command;
  uint8_t test_buffer[4] = { 0 };

  /* Read the status register command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_MULTIPLE_IO_READ_ID_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_4_LINES;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_4_LINES;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 3;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
  return 1;
  }

  if (HAL_OSPI_Receive(&hospi2, test_buffer, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }
  return status;
}

/* This function enables the qpi mode of the memory */
static uint8_t OSPI_SetQpiMode(void)
{
  uint8_t status = 0;
  OSPI_RegularCmdTypeDef s_command;
  OSPI_AutoPollingTypeDef s_config;

  /* Read the status register command */
  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_ENABLE_QSPI_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_NONE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 0;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
  return 1;
  }

  /* Configure automatic polling mode to wait for QPI enabling */
  s_config.MatchMode              = HAL_OSPI_MATCH_MODE_AND;
  s_config.Interval               = 0x10;
  s_config.AutomaticStop          = HAL_OSPI_AUTOMATIC_STOP_ENABLE;
  s_config.Match                  = MX25L512_SR_QUADEN;
  s_config.Mask                   = MX25L512_SR_QUADEN;

  s_command.OperationType         = HAL_OSPI_OPTYPE_COMMON_CFG;
  s_command.FlashId               = HAL_OSPI_FLASH_ID_1;
  s_command.Instruction           = MX25L512_READ_STATUS_REG_CMD;
  s_command.InstructionMode       = HAL_OSPI_INSTRUCTION_1_LINE;
  s_command.InstructionSize       = HAL_OSPI_INSTRUCTION_8_BITS;
  s_command.InstructionDtrMode    = HAL_OSPI_INSTRUCTION_DTR_DISABLE;
  s_command.AddressMode           = HAL_OSPI_ADDRESS_NONE;
  s_command.AlternateBytesMode    = HAL_OSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode              = HAL_OSPI_DATA_1_LINE;
  s_command.DummyCycles           = 0;
  s_command.DQSMode               = HAL_OSPI_DQS_DISABLE;
  s_command.SIOOMode              = HAL_OSPI_SIOO_INST_EVERY_CMD;
  s_command.NbData                = 1;
  s_command.DataDtrMode           = HAL_OSPI_DATA_DTR_DISABLE;

  if (HAL_OSPI_Command(&hospi2, &s_command, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  if (HAL_OSPI_AutoPolling(&hospi2, &s_config, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return 1;
  }

  return status;
}

/**
  * @brief  Weak MX init (user overrideable).
  */
__weak HAL_StatusTypeDef MX_OSPI_Init(OSPI_HandleTypeDef *hospi)
{

  /* OCTOSPI2 parameter configuration*/
  hospi->Instance = OCTOSPI2;
  hospi->Init.FifoThreshold = 1;
  hospi->Init.DualQuad = HAL_OSPI_DUALQUAD_DISABLE;
  hospi->Init.MemoryType = HAL_OSPI_MEMTYPE_MACRONIX;
  hospi->Init.DeviceSize = 26;
  hospi->Init.ChipSelectHighTime = 2;
  hospi->Init.FreeRunningClock = HAL_OSPI_FREERUNCLK_DISABLE;
  hospi->Init.ClockMode = HAL_OSPI_CLOCK_MODE_0;
  hospi->Init.WrapSize = HAL_OSPI_WRAP_NOT_SUPPORTED;
  hospi->Init.ClockPrescaler = 4;
  hospi->Init.SampleShifting = HAL_OSPI_SAMPLE_SHIFTING_NONE;
  hospi->Init.DelayHoldQuarterCycle = HAL_OSPI_DHQC_DISABLE;
  hospi->Init.ChipSelectBoundary = 0;
  hospi->Init.DelayBlockBypass = HAL_OSPI_DELAY_BLOCK_BYPASSED;
  hospi->Init.MaxTran = 0;
  hospi->Init.Refresh = 0;
  if (HAL_OK != HAL_OSPI_Init(hospi))
  {
    return HAL_OSPI_ERROR_TRANSFER;
  }

  OCTOSPI2->CR &= ~(1 << 24);

  /*## IOM configuration ##*/

  CLEAR_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);

  MODIFY_REG(OCTOSPIM->CR,
    (OCTOSPIM_CR_MUXEN | OCTOSPIM_CR_MODE | OCTOSPIM_CR_CSSEL_OVR_EN
        | OCTOSPIM_CR_CSSEL_OVR_O1 | OCTOSPIM_CR_CSSEL_OVR_O2),
    (HAL_OSPIM_MUX_DISABLE | HAL_OSPIM_MODE_SWAPPED
        | HAL_OSPIM_CSSEL_OVR_DISABLE | HAL_OSPIM_CSSEL_OVR_01_NCS1
        | HAL_OSPIM_CSSEL_OVR_02_NCS1));

  SET_BIT(OCTOSPI2->CR, OCTOSPI_CR_EN);

  return HAL_OSPI_ERROR_NONE;
}

/**
* @brief OSPI MSP Initialization
* This function configures the hardware resources used
* @param hospi: OSPI handle pointer
* @retval None
*/
static void OSPI_MspInit(OSPI_HandleTypeDef* hospi)
{
  if(OCTOSPI2 == hospi->Instance)
  {
    /* USER CODE BEGIN OCTOSPI2_MspInit 0 */
    /* Acquire OCTOSPI2 peripheral via Resource Manager */
    if (RESMGR_STATUS_ACCESS_OK != ResMgr_Request(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_OCTOSPI2_ID))
    {
      Error_Handler();
    }

    /* Acquire OCTOSPI IO Manager via Resource Manager */
    if (RESMGR_STATUS_ACCESS_OK != ResMgr_Request(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_OCTOSPIM_ID))
    {
      Error_Handler();
    }

    /* Acquire GPIOD pins used by OCTOSPI2: PD0, PD3, PD4, PD5, PD6, PD7 */
    if (ResMgr_GPIO_Request(RESMGR_RESOURCE_RIF_GPIOD,
                            GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
        != RESMGR_STATUS_ACCESS_OK)
    {
      Error_Handler();
    }

#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Acquire RCC resource for GPIOD bank clock */
    if (RESMGR_STATUS_ACCESS_OK == ResMgr_Request(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(93)))
    {
      __HAL_RCC_GPIOD_CLK_ENABLE();
    }

    /* Ensure VDDIO3 supply for GPIOD bank is ON */
    HAL_PWREx_EnableSupplyMonitoring(PWR_PVM_VDDIO3);
    HAL_PWREx_EnableSupply(PWR_PVM_VDDIO3);
    while (HAL_PWREx_PVM_IsBelowThreshold(PWR_PVM_VDDIO3));

#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* USER CODE END OCTOSPI2_MspInit 0 */

    /**OCTOSPI2 GPIO Configuration
    PD0     ------> OCTOSPIM_P1_CLK
    PD4     ------> OCTOSPIM_P1_IO0
    PD5     ------> OCTOSPIM_P1_IO1
    PD6     ------> OCTOSPIM_P1_IO2
    PD7     ------> OCTOSPIM_P1_IO3
    PD3     ------> OCTOSPIM_P1_NCS
    */

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* Peripheral clock enable */

    /* Enable clock and Reset the OctoSPI IO Manager interface */
    __HAL_RCC_OSPIIOM_CLK_ENABLE();
    __HAL_RCC_OSPIIOM_CLK_SLEEP_ENABLE();
    __HAL_RCC_OSPIIOM_FORCE_RESET();
    __HAL_RCC_OSPIIOM_RELEASE_RESET();

    /* Enable the OctoSPI memory interface clock and reset */
    __HAL_RCC_OSPI2_CLK_ENABLE();
    __HAL_RCC_OSPI2_FORCE_RESET();
    __HAL_RCC_OSPI2_RELEASE_RESET();

    /* OCTOSPI2 interrupt Init */
    HAL_NVIC_SetPriority(OCTOSPI2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OCTOSPI2_IRQn);


    /* USER CODE BEGIN OCTOSPI2_MspInit 1 */

    /* USER CODE END OCTOSPI2_MspInit 1 */
  }
}

/**
* @brief OSPI MSP De-Initialization
* This function freeze the hardware resources used
* @param hospi: OSPI handle pointer
* @retval None
*/
static void OSPI_MspDeInit(OSPI_HandleTypeDef* hospi)
{
  if(hospi->Instance == OCTOSPI2)
  {
    /* USER CODE BEGIN OCTOSPI2_MspDeInit 0 */

    /* OCTOSPI2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(OCTOSPI2_IRQn);

    /* Reset the OctoSPI memory interface */
    __HAL_RCC_OSPI2_FORCE_RESET();
    __HAL_RCC_OSPI2_RELEASE_RESET();

    /* DeInit GPIODs pins */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 |
        GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    /* Peripheral clock disable */
    __HAL_RCC_OSPI2_CLK_DISABLE();

    /* USER CODE END OCTOSPI2_MspDeInit 0 */


    /* USER CODE BEGIN OCTOSPI2_MspDeInit 1 */

    /* Release GPIOD pins reserved for OCTOSPI2 */
    (void)ResMgr_GPIO_Release(RESMGR_RESOURCE_RIF_GPIOD,
                              GPIO_PIN_0 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

#if !defined(A35_STARTUP_IN_ARM_MODE)
    /* Release RCC resource used to enable GPIOD clock */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIF_RCC, RESMGR_RCC_RESOURCE(93));
#endif /*!A35_STARTUP_IN_ARM_MODE*/

    /* Release OCTOSPI2 RIFSC reservation */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_OCTOSPI2_ID);

    /* Release OCTOSPI IO Manager RIFSC reservation  */
    (void)ResMgr_Release(RESMGR_RESOURCE_RIFSC, STM32MP25_RIFSC_OCTOSPIM_ID);

    /* USER CODE END OCTOSPI2_MspDeInit 1 */
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
