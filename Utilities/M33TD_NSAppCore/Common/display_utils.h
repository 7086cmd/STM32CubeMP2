/**
******************************************************************************
* @file    display_utils.h
* @author  MCD Application Team
* @brief   Display utilities API (framebuffer, LTDC, palette)
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
#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported macros -----------------------------------------------------------*/
/* Little-endian read helpers */
#define DISPLAY_UTILS_LE16(p) ((uint16_t)(                         \
                                 ((uint16_t)((const uint8_t *)(p))[0]) | \
                                 ((uint16_t)((const uint8_t *)(p))[1] << 8)))

#define DISPLAY_UTILS_LE32(p) ((uint32_t)(                          \
                                 ((uint32_t)((const uint8_t *)(p))[0]) | \
                                 ((uint32_t)((const uint8_t *)(p))[1] << 8) | \
                                 ((uint32_t)((const uint8_t *)(p))[2] << 16) | \
                                 ((uint32_t)((const uint8_t *)(p))[3] << 24)))

/* Exported functions --------------------------------------------------------*/

/**
 * @brief  Check whether the LTDC clock is enabled.
 * @retval true if enabled, false otherwise.
 */
bool DisplayUtils_IsLtdcClockEnabled(void);

/**
 * @brief  Decode an 8bpp RLE-compressed BMP image to a framebuffer.
 * @param  bmp_data   Pointer to BMP file data in memory
 * @param  fb         Pointer to framebuffer
 * @param  fb_width   Framebuffer width in pixels
 * @param  fb_height  Framebuffer height in pixels
 * @retval 0 on success, -1 on error
 */
int DisplayUtils_DecodeSplashBMP(const unsigned char *bmp_data, uint8_t *fb, int fb_width, int fb_height);

/**
 * @brief  Configure the LTDC layer for a given framebuffer region.
 * @param  fb Pointer to framebuffer
 * @param  x  X position of region
 * @param  y  Y position of region
 * @param  w  Width of region
 * @param  h  Height of region
 * @retval None
 */
void DisplayUtils_LTDC_ConfigLayer(uint8_t *fb, int x, int y, int w, int h);

/**
 * @brief  Hide the LTDC layer.
 * @retval None
 */
void DisplayUtils_LTDC_HideLayer(void);

/**
 * @brief  Swap double buffers, update LTDC framebuffer address with VBR reload,
 *         and optionally update the color palette (CLUT) if update_palette is true.
 * @param  fb1            Pointer to pointer to the currently displayed framebuffer (will be updated)
 * @param  fb2            Pointer to pointer to the back/decode framebuffer (will be updated)
 * @param  layer          LTDC layer index (e.g., LTDC_LAYER_3)
 * @param  update_palette If true, update the CLUT using the internal palette buffer
 * @retval None
 */
void DisplayUtils_FlipBuffers(uint8_t **fb1, uint8_t **fb2, int layer, bool update_palette);

#endif /* DISPLAY_UTILS_H */

