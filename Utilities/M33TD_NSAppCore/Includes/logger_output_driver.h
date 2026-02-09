/**
  ******************************************************************************
  * @file    logger_output_driver.h
  * @author  Application Team
  * @brief   Logger output driver interface (only init exposed).
  ******************************************************************************
  */

#ifndef LOGGER_OUTPUT_DRIVER_H
#define LOGGER_OUTPUT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @addtogroup LoggerOutputDriver
  * @{
  */

/**
  * @brief  Initialize the logger output (e.g., UART, ITM, etc.).
  *         This function is called by LoggerTask_Init().
  * @retval None
  */
void logger_output_init(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_OUTPUT_DRIVER_H */