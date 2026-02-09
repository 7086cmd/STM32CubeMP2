/**
  ******************************************************************************
  * @file    lcd_panel_rk043fn48h.h
  * @author  MCD Application Team
  * @brief   LCD timing/constants for RK043FN48H (RGB, portrait-only).
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_RGB_PANEL_RK043FN48H_H
#define __LCD_RGB_PANEL_RK043FN48H_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/*
 * Rocktech RK043FN48H (480x272)
 * Timing values are derived from the BSP component header rk043fn48h.h.
 */
#define RK043FN48H_WIDTH   ((uint16_t)480)
#define RK043FN48H_HEIGHT  ((uint16_t)272)

#define RK043FN48H_HSYNC   ((uint16_t)41)
#define RK043FN48H_HBP     ((uint16_t)13)
#define RK043FN48H_HFP     ((uint16_t)32)
#define RK043FN48H_VSYNC   ((uint16_t)10)
#define RK043FN48H_VBP     ((uint16_t)2)
#define RK043FN48H_VFP     ((uint16_t)2)

/* Portrait only (project must not select landscape). */
#if defined(LCD_ORIENTATION_LANDSCAPE)
#error "RK043FN48H panel supports portrait-only in this utility stack header."
#endif

/* LCD Timing (portrait) */
#define LCD_WIDTH   RK043FN48H_WIDTH
#define LCD_HEIGHT  RK043FN48H_HEIGHT

#define LCD_HSYNC   RK043FN48H_HSYNC
#define LCD_HBP     RK043FN48H_HBP
#define LCD_HFP     RK043FN48H_HFP
#define LCD_VSYNC   RK043FN48H_VSYNC
#define LCD_VBP     RK043FN48H_VBP
#define LCD_VFP     RK043FN48H_VFP

/* Pixel format (RGB) */
#define LCD_FORMAT_RGB888  ((uint32_t)0x00) /* 24 bpp */

#endif /* __LCD_RGB_PANEL_RK043FN48H_H */

