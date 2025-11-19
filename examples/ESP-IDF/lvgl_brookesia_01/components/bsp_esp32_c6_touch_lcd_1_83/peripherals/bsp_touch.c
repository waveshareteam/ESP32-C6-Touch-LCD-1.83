#include "bsp_board.h"
#include "driver/i2c_master.h"
#include "esp_lcd_touch_cst816s.h"

static esp_lcd_touch_handle_t tp_handle = NULL;


esp_err_t bsp_touch_driver_init(void)
{
    esp_err_t ret = ESP_OK;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;

    
    i2c_master_bus_handle_t i2c_handle;
    i2c_master_get_bus_handle(0,&i2c_handle);
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = EXAMPLE_LCD_H_RES,
        .y_max = EXAMPLE_LCD_V_RES,
        .rst_gpio_num = TOUCH_RST,
        .int_gpio_num = TOUCH_INT,
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((i2c_master_bus_handle_t)i2c_handle, &tp_io_config, &tp_io_handle));
    ret = esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp_handle);

    bsp_handles_t *handles = bsp_display_get_handles();
    handles->tp_handle = tp_handle;
    return ret;
}