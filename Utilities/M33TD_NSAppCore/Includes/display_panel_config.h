/**
  ******************************************************************************
  * @file    display_panel_config.h
  * @author  MCD Application Team
  * @brief   Public interface for project-side panel configuration.
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

#ifndef DISPLAY_PANEL_CONFIG_H
#define DISPLAY_PANEL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Panel types.
  * @note  Project may extend this enum with additional panels.
  */
typedef enum
{
  DISPLAY_PANEL_FHD = 0,
  DISPLAY_PANEL_ROCKTECH,
  DISPLAY_PANEL_OLED,
} display_panel_type_t;

/**
  * @brief Display Panel configuration.
  *
  * Project must define one instance of this struct per supported panel,
  * and provide the active panel by defining the global symbol:
  * `const display_panel_config_t *display_active_panel;`
  */
typedef struct
{
  display_panel_type_t type;
  const char *name;

  int width;
  int height;

  void (*init)(void);
  void (*power_on)(void);
  void (*power_off)(void);

#if defined(SPLASH_ANIMATION_ENABLED)
  const unsigned char **animation_frames;
  int animation_frame_count;
  int animation_fps;
#else
  const unsigned char *splash_bmp;
#endif

} display_panel_config_t;

/**
  * @brief Active panel selection.
  * @note  Project must define this symbol in exactly one C file.
  */
extern const display_panel_config_t *display_active_panel;

#ifdef __cplusplus
}
#endif

#endif /* DISPLAY_PANEL_CONFIG_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

