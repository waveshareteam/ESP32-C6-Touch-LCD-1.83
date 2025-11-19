#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/i2c_master.h"

#include "lcd_driver.h"
#include "lvgl_driver.h"
#include "touch_driver.h"
#include "lv_demos.h"

static const char *TAG = "app_main";




static esp_err_t i2c_master_init(void)
{
    i2c_master_bus_handle_t i2c_bus_handle;
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = 0,
        .sda_io_num = GPIO_NUM_7,
        .scl_io_num = GPIO_NUM_8,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C bus initialized successfully");
    return ESP_OK;  // 返回 ESP_OK 表示成功
}

extern "C" void app_main(void)
{
    i2c_master_init();

    lcd_driver_init();
    touch_driver_init();
    lvgl_driver_init();
    
    lvgl_port_lock(0);
    lv_demo_stress();
    lvgl_port_unlock();

}