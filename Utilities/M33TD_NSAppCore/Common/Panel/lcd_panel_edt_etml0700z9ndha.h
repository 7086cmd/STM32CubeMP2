/**
  ******************************************************************************
  * @file    lcd_panel_edt_etml0700z9ndha.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    18-February-2014
  * @brief   This file contains all the constants parameters for the EDT ETML0700Z9NDHA LVDS
  *          LCD component.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_LVDS_PANEL_EDT_ETML0700Z9NDHA_H
#define __LCD_LVDS_PANEL_EDT_ETML0700Z9NDHA_H

/* Includes ------------------------------------------------------------------*/

/**
 *  @brief  Possible values of
 *  pixel data format (ie color coding) transmitted on LVDS Data lane in LVDS packets
 */
#define EDT_ETML0700Z9NDHA_FORMAT_RGB888    ((uint32_t)0x00) /* Pixel format chosen is RGB888 : 24 bpp */
#define EDT_ETML0700Z9NDHA_FORMAT_RBG666    ((uint32_t)0x02) /* Pixel format chosen is RGB666 : 18 bpp */

#define EDT_ETML0700Z9NDHA_LCD_ID  ((uint32_t) 0)

/**
  * @brief  EDT_ETML0700Z9NDHA_1024X600 Size
  */

/* Width and Height in Portrait mode */
#define  EDT_ETML0700Z9NDHA_1024X600_WIDTH             ((uint16_t)1024)     /* LCD PIXEL WIDTH   */
#define  EDT_ETML0700Z9NDHA_1024X600_HEIGHT            ((uint16_t)600)      /* LCD PIXEL HEIGHT  */

/* Width and Height in Landscape mode */
#define  EDT_ETML0700Z9NDHA_600X1024_WIDTH             ((uint16_t)600)      /* LCD PIXEL WIDTH   */
#define  EDT_ETML0700Z9NDHA_600X1024_HEIGHT            ((uint16_t)1024)     /* LCD PIXEL HEIGHT  */

/**
  * @brief  EDT_ETML0700Z9NDHA_1024X600 Timing parameters for Portrait orientation mode
  */
#define  EDT_ETML0700Z9NDHA_1024X600_HSYNC             ((uint16_t)21)      /* Horizontal synchronization */
#define  EDT_ETML0700Z9NDHA_1024X600_HBP               ((uint16_t)150)     /* Horizontal back porch      */
#define  EDT_ETML0700Z9NDHA_1024X600_HFP               ((uint16_t)150)     /* Horizontal front porch     */
#define  EDT_ETML0700Z9NDHA_1024X600_VSYNC             ((uint16_t)21)      /* Vertical synchronization   */
#define  EDT_ETML0700Z9NDHA_1024X600_VBP               ((uint16_t)24)      /* Vertical back porch        */
#define  EDT_ETML0700Z9NDHA_1024X600_VFP               ((uint16_t)24)      /* Vertical front porch       */

/**
  * @brief  EDT_ETML0700Z9NDHA_600X1024 Timing parameters for Landscape orientation mode
  *         Same values as for Portrait mode in fact.
  */
#define  EDT_ETML0700Z9NDHA_600X1024_HSYNC             EDT_ETML0700Z9NDHA_1024X600_VSYNC  /* Horizontal synchronization */
#define  EDT_ETML0700Z9NDHA_600X1024_HBP               EDT_ETML0700Z9NDHA_1024X600_VBP    /* Horizontal back porch      */
#define  EDT_ETML0700Z9NDHA_600X1024_HFP               EDT_ETML0700Z9NDHA_1024X600_VFP    /* Horizontal front porch     */
#define  EDT_ETML0700Z9NDHA_600X1024_VSYNC             EDT_ETML0700Z9NDHA_1024X600_HSYNC  /* Vertical synchronization   */
#define  EDT_ETML0700Z9NDHA_600X1024_VBP               EDT_ETML0700Z9NDHA_1024X600_HBP    /* Vertical back porch        */
#define  EDT_ETML0700Z9NDHA_600X1024_VFP               EDT_ETML0700Z9NDHA_1024X600_HFP    /* Vertical front porch       */

/**
  * @brief  LCD Timing
  */
#if defined(LCD_ORIENTATION_PORTRAIT)
#define  LCD_WIDTH         EDT_ETML0700Z9NDHA_1024X600_WIDTH     /* LCD PIXEL WIDTH            */
#define  LCD_HEIGHT        EDT_ETML0700Z9NDHA_1024X600_HEIGHT    /* LCD PIXEL HEIGHT           */

#define  LCD_HSYNC         EDT_ETML0700Z9NDHA_1024X600_HSYNC     /* Horizontal synchronization */
#define  LCD_HBP           EDT_ETML0700Z9NDHA_1024X600_HBP       /* Horizontal back porch      */
#define  LCD_HFP           EDT_ETML0700Z9NDHA_1024X600_HFP       /* Horizontal front porch     */
#define  LCD_VSYNC         EDT_ETML0700Z9NDHA_1024X600_VSYNC     /* Vertical synchronization   */
#define  LCD_VBP           EDT_ETML0700Z9NDHA_1024X600_VBP       /* Vertical back porch        */
#define  LCD_VFP           EDT_ETML0700Z9NDHA_1024X600_VFP       /* Vertical front porch       */

#elif defined(LCD_ORIENTATION_LANDSCAPE)
#define  LCD_WIDTH         EDT_ETML0700Z9NDHA_600X1024_WIDTH     /* LCD PIXEL WIDTH            */
#define  LCD_HEIGHT        EDT_ETML0700Z9NDHA_600X1024_HEIGHT    /* LCD PIXEL HEIGHT           */

#define  LCD_HSYNC         EDT_ETML0700Z9NDHA_600X1024_HSYNC     /* Horizontal synchronization */
#define  LCD_HBP           EDT_ETML0700Z9NDHA_600X1024_HBP       /* Horizontal back porch      */
#define  LCD_HFP           EDT_ETML0700Z9NDHA_600X1024_HFP       /* Horizontal front porch     */
#define  LCD_VSYNC         EDT_ETML0700Z9NDHA_600X1024_VSYNC     /* Vertical synchronization   */
#define  LCD_VBP           EDT_ETML0700Z9NDHA_600X1024_VBP       /* Vertical back porch        */
#define  LCD_VFP           EDT_ETML0700Z9NDHA_600X1024_VFP       /* Vertical front porch       */
#else
#error "Please select the orientation  LCD_ORIENTATION_PORTRAIT or LCD_ORIENTATION_LANDSCAPE"
#endif

#define LCD_FORMAT_RGB888 EDT_ETML0700Z9NDHA_FORMAT_RGB888   /* Pixel format chosen is RGB888 : 24 bpp */
#define LCD_FORMAT_RGB666 EDT_ETML0700Z9NDHA_FORMAT_RBG666   /* Pixel format chosen is RGB666 : 18 bpp */

/**
  * @brief  LCD Display EDT_ETML0700Z9NDHA ID
  */

#define LCD_LVDS_ID  EDT_ETML0700Z9NDHA_LCD_ID

#endif /* __LCD_LVDS_PANEL_EDT_ETML0700Z9NDHA_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

