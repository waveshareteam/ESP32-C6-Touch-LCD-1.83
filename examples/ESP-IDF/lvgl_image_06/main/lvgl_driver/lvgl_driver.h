#pragma once

#include "esp_lvgl_port.h"

#ifdef __cplusplus
extern "C" {
#endif

extern lv_display_t *lvgl_disp;
extern lv_indev_t *lvgl_touch_indev;

esp_err_t lvgl_driver_init(void);

#ifdef __cplusplus
}
#endif
