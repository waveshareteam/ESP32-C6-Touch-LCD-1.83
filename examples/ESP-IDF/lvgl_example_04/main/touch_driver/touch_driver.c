#include "touch_driver.h"

#include "sdkconfig.h"
#include "lcd_driver.h"
#include "driver/i2c_master.h"

static const char *TAG = "touch driver";

esp_lcd_touch_handle_t tp_handle = NULL;


void touch_driver_init(void)
{
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
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &tp_handle));



}