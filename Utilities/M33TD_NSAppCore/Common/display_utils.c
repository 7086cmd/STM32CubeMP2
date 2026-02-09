/**
******************************************************************************
* @file    display_utils.c
* @author  MCD Application Team
* @brief   Display utilities functions (framebuffer, LTDC, palette)
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
#include "display_utils.h"
#include "nsappcore_config.h"

#include <string.h>

/* External variables --------------------------------------------------------*/
/** @brief LTDC handler (external). */
extern LTDC_HandleTypeDef hLtdcHandler;

/* Private defines -----------------------------------------------------------*/

/** @brief 8bpp RLE compression type for BMP. */
#define DISPLAY_UTILS_BI_RLE8 1U

/** @brief Number of palette entries for 8bpp images. */
#define DISPLAY_UTILS_PALETTE_SIZE 256U

/* Private types -------------------------------------------------------------*/

/**
 * @brief  BMP file header structure (packed).
 */
#pragma pack(push, 1)
typedef struct {
    char signature[2];           /**< File signature, should be 'BM' */
    uint32_t file_size;          /**< Size of the BMP file in bytes */
    uint32_t reserved;           /**< Reserved, must be zero */
    uint32_t data_offset;        /**< Offset to start of pixel data */
    uint32_t size;               /**< Size of this header (40 bytes) */
    int32_t width;               /**< Image width in pixels */
    int32_t height;              /**< Image height in pixels */
    uint8_t unused[32];          /**< Unused/ignored fields (BITMAPINFOHEADER) */
} DisplayUtils_BmpHeader_t;
#pragma pack(pop)

/**
 * @brief  BMP color table entry (palette).
 */
typedef struct {
    uint8_t b; /**< Blue component */
    uint8_t g; /**< Green component */
    uint8_t r; /**< Red component */
    uint8_t x; /**< Reserved (unused) */
} DisplayUtilsColor_t;

/* Private variables ---------------------------------------------------------*/

/** @brief Static color palette buffer for display (BGRA8888 bytes). */
static DisplayUtilsColor_t display_utils_palette[DISPLAY_UTILS_PALETTE_SIZE];

/* Exported functions --------------------------------------------------------*/

/**
    * @brief  Check whether the LTDC clock is enabled.
    * @retval true if enabled, false otherwise.
    */
bool DisplayUtils_IsLtdcClockEnabled(void)
{
    return (__HAL_RCC_LTDC_IS_CLK_ENABLED() != 0U);
}

/**
    * @brief  Decode an 8bpp RLE-compressed BMP image to a framebuffer.
    * @param  bmp_data   Pointer to BMP file data in memory
    * @param  fb         Pointer to framebuffer
    * @param  fb_width   Framebuffer width in pixels
    * @param  fb_height  Framebuffer height in pixels
    * @retval 0 on success, -1 on error
    */
int DisplayUtils_DecodeSplashBMP(const unsigned char *bmp_data, uint8_t *fb, int fb_width, int fb_height)
{
    const DisplayUtils_BmpHeader_t *bmpHeader = (const DisplayUtils_BmpHeader_t *)bmp_data;

    const int bmp_width = (int)DISPLAY_UTILS_LE32(&bmpHeader->width);
    const int bmp_height = (int)DISPLAY_UTILS_LE32(&bmpHeader->height);
    const uint32_t headerSize = DISPLAY_UTILS_LE32(&bmpHeader->size);
    /*
     * Palette handling assumptions:
     * - Input is an 8bpp indexed BMP used for splash screens.
     * - A full 256-entry palette (BGRA) is present immediately after the DIB header.
     */
    const DisplayUtilsColor_t *bmp_palette = (const void *)(bmp_data + 14U + headerSize);
    memcpy(display_utils_palette, bmp_palette, DISPLAY_UTILS_PALETTE_SIZE * sizeof(DisplayUtilsColor_t));

    /* Start of pixel data. */
    const uint8_t *bmpPtr = bmp_data + DISPLAY_UTILS_LE32(&bmpHeader->data_offset);

    int posX = 0;
    int posY = bmp_height - 1;
    const int width = bmp_width;
    /* fb_row points to the first pixel of row posY in the destination framebuffer.
     * Note: for an L8 framebuffer, fb_width is the row stride in pixels.
     */
    uint8_t *fb_row = fb + posY * fb_width;

    while (posY >= 0) {
        uint8_t count = *bmpPtr++;
        if (count == 0) {
            uint8_t cmd = *bmpPtr++;
            if (cmd == 0) {
                /* End of line */
                posX = 0;
                posY--;
                if (posY < 0) break;
                /* Move to the start of the next row in the framebuffer (one row up). */
                fb_row = fb + posY * fb_width;
            } else if (cmd == 1) {
                /* End of bitmap */
                break;
            } else if (cmd == 2) {
                /* Delta */
                uint8_t stepX = *bmpPtr++;
                uint8_t stepY = *bmpPtr++;
                posX += stepX;
                posY -= stepY;
                if (posY < 0) break;
                /* Update row pointer after cursor moves; column is handled by fb_row[posX]. */
                fb_row = fb + posY * fb_width;
            } else {
                /* Absolute mode */
                int n = cmd;
                for (int i = 0; i < n && posX < width; i++, posX++)
                {
                    if (posX < fb_width && posY < fb_height && posX >= 0 && posY >= 0)
                    {
                        fb_row[posX] = *bmpPtr++;
                    }
                }
                if (n & 1) bmpPtr++; /* Pad to even */
            }
        } else {
            /* Encoded mode */
            uint8_t val = *bmpPtr++;
            for (int i = 0; i < count && posX < width; i++, posX++)
            {
                if (posX < fb_width && posY < fb_height && posX >= 0 && posY >= 0)
                {
                    fb_row[posX] = val;
                }
            }
        }
    }

    return 0;
}

/**
    * @brief  Configure the LTDC layer for a given framebuffer region.
    * @param  fb Pointer to framebuffer
    * @param  x  X position of region
    * @param  y  Y position of region
    * @param  w  Width of region
    * @param  h  Height of region
    * @retval None
    */
void DisplayUtils_LTDC_ConfigLayer(uint8_t *fb, int x, int y, int w, int h)
{
    LTDC_LayerCfgTypeDef layer_cfg;
    memset(&layer_cfg, 0, sizeof(LTDC_LayerCfgTypeDef));

    layer_cfg.WindowX0 = x;
    layer_cfg.WindowY0 = y;
    layer_cfg.WindowX1 = x + w;
    layer_cfg.WindowY1 = y + h;
    layer_cfg.ImageWidth = w;
    layer_cfg.ImageHeight = h;
    layer_cfg.FBStartAdress = (uint32_t)fb;
    layer_cfg.Alpha = 0xf0;
    layer_cfg.Alpha0 = 0;
    layer_cfg.Backcolor.Blue = 0xff;
    layer_cfg.Backcolor.Green = 0xff;
    layer_cfg.Backcolor.Red = 0xff;
    layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
    layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
    layer_cfg.HorMirrorEn = 0;
    layer_cfg.VertMirrorEn = 0;
    layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_L8;

    HAL_LTDC_ConfigLayer_NoReload(&hLtdcHandler, &layer_cfg, LTDC_LAYER_3);
    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->RCR = LTDC_LxRCR_IMR;

    /* Use the static palette (captured during decode). */
    HAL_LTDC_ConfigCLUT(&hLtdcHandler, (uint32_t *)display_utils_palette, DISPLAY_UTILS_PALETTE_SIZE, LTDC_LAYER_3);
    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->CR |= (uint32_t)LTDC_LxCR_CLUTEN;

    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->RCR = LTDC_LxRCR_IMR;
}

/**
    * @brief  Hide the LTDC layer.
    * @retval None
    */
void DisplayUtils_LTDC_HideLayer(void)
{
    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->CR &= ~LTDC_LxCR_LEN;
    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->CFBAR = 0x0;
    LTDC_LAYER(&hLtdcHandler, LTDC_LAYER_3)->RCR = LTDC_LxRCR_IMR;
}

/**
    * @brief  Swap double buffers, update LTDC framebuffer address with VBR reload,
    *         and optionally update the color palette (CLUT) if update_palette is true.
    * @param  fb1            Pointer to pointer to the currently displayed framebuffer (will be updated)
    * @param  fb2            Pointer to pointer to the back/decode framebuffer (will be updated)
    * @param  layer          LTDC layer index (e.g., LTDC_LAYER_3)
    * @param  update_palette If true, update the CLUT using the internal palette buffer
    * @retval None
    */
void DisplayUtils_FlipBuffers(uint8_t **fb1, uint8_t **fb2, int layer, bool update_palette)
{
    if (update_palette)
    {
        HAL_LTDC_ConfigCLUT(&hLtdcHandler, (uint32_t *)display_utils_palette, DISPLAY_UTILS_PALETTE_SIZE, layer);
        LTDC_LAYER(&hLtdcHandler, layer)->CR |= (uint32_t)LTDC_LxCR_CLUTEN;
    }

    /* Swap pointers. */
    uint8_t *tmp = *fb1;
    *fb1 = *fb2;
    *fb2 = tmp;

    /* Update LTDC framebuffer address and trigger VBR reload. */
    LTDC_LAYER(&hLtdcHandler, layer)->CFBAR = (uint32_t)(*fb1);
    LTDC_LAYER(&hLtdcHandler, layer)->RCR = LTDC_LxRCR_VBR;
}

