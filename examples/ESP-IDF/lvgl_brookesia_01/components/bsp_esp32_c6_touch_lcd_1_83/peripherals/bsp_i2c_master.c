#include "bsp_board.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

static const char *TAG = "bsp i2c";

esp_err_t bsp_i2c_master_init(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM,
        .sda_io_num = GPIO_I2C_SDA,
        .scl_io_num = GPIO_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };

    i2c_master_bus_handle_t i2c_bus;
    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C bus initialized successfully");
    return ESP_OK;  // 返回 ESP_OK 表示成功
}