#include "bsp_board.h"
#include "esp_log.h"
#include "esp_check.h"


static const char *TAG = "bsp lvgl driver";

/* LVGL display and touch */
 lv_display_t *lvgl_disp = NULL;
 lv_indev_t *lvgl_touch_indev = NULL;

esp_err_t lvgl_driver_init(void)
{
    esp_lcd_panel_io_handle_t lcd_io;
    esp_lcd_panel_handle_t lcd_panel; 
    esp_lcd_touch_handle_t tp_handle;
    bsp_handles_t *handles = bsp_display_get_handles();
    lcd_io = handles->io;
    lcd_panel = handles->panel;
    tp_handle = handles->tp_handle;

    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = {
        .task_priority = 3,         /* LVGL task priority */
        .task_stack = 8196,         /* LVGL task stack size */
        .task_affinity = -1,        /* LVGL task pinned to core (-1 is no affinity) */
        .task_max_sleep_ms = 1000,   /* Maximum sleep in LVGL task */
        .timer_period_ms = 10        /* LVGL timer tick period in ms */
    };
    ESP_RETURN_ON_ERROR(lvgl_port_init(&lvgl_cfg), TAG, "LVGL port initialization failed");

    /* Add LCD screen */
    ESP_LOGD(TAG, "Add LCD screen");
    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = lcd_io,
        .panel_handle = lcd_panel,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_DRAW_BUFF_HEIGHT,
        .double_buffer = EXAMPLE_LCD_DRAW_BUFF_DOUBLE,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,


        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .swap_bytes = true,
            .buff_spiram = false
        }
    };
    lvgl_disp = lvgl_port_add_disp(&disp_cfg);

    /* Add touch input (for selected screen) */
    const lvgl_port_touch_cfg_t touch_cfg = {
        .disp = lvgl_disp,
        .handle = tp_handle,
    };
    lvgl_touch_indev = lvgl_port_add_touch(&touch_cfg);

    handles->lvgl_disp_handle = lvgl_disp;
    handles->lvgl_touch_indev_handle = lvgl_touch_indev;
    return ESP_OK;
}