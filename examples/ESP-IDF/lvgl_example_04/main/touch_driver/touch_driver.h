#pragma once

#include "sdkconfig.h"

#include "esp_lcd_touch_cst816s.h"


#ifdef __cplusplus
extern "C" {
#endif

extern esp_lcd_touch_handle_t tp_handle;

void touch_driver_init(void);

#ifdef __cplusplus
}
#endif
