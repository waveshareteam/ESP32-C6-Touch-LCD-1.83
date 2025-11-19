#include "bsp_board.h"
#include "nvs_flash.h"

static bsp_handles_t g_lcd_handles = {
    .panel = NULL,
    .io = NULL,
    .tp_handle = NULL,
    .i2s_rx_handle = NULL,
    .i2s_tx_handle = NULL,
    .lvgl_disp_handle = NULL,
    .lvgl_touch_indev_handle = NULL
};



esp_err_t bsp_init(void)
{
        esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(bsp_i2c_master_init());
    ESP_ERROR_CHECK(bsp_i2s_init());
    ESP_ERROR_CHECK(bsp_codec_init());
    ESP_ERROR_CHECK(bsp_lcd_driver_init());
    ESP_ERROR_CHECK(bsp_touch_driver_init());
    sd_card_init();
    ESP_ERROR_CHECK(qmi8658_driver_init());
    ESP_ERROR_CHECK(pcf85063a_driver_init());
    ESP_ERROR_CHECK(lvgl_driver_init());

    return ESP_OK;
}

// get lcd and touch handle
bsp_handles_t *bsp_display_get_handles(void)
{
    return &g_lcd_handles;
}