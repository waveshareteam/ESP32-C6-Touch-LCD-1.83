#include "bsp_board.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"


static const char *TAG = "bsp rtc";

#define RTC_INT_PIN          GPIO_NUM_15

// Initial RTC time to be set
static pcf85063a_datetime_t Set_Time = {
    .year = 2025,
    .month = 10,
    .day = 30,
    .dotw = 0,   // Day of the week: 0 = Sunday
    .hour = 0,
    .min = 0,
    .sec = 0
};


static pcf85063a_dev_t dev;

static esp_err_t gpio_int_init(void)
{
    // Zero-initialize the GPIO configuration structure
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE; // Disable interrupts for this pin
    io_conf.pin_bit_mask = 1ULL << RTC_INT_PIN;    // Select the GPIO pin using a bitmask

    io_conf.mode = GPIO_MODE_INPUT;          // Set pin as input
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // Enable internal pull-up resistor


    return gpio_config(&io_conf); // Apply the configuration
}


esp_err_t pcf85063a_driver_init(void)
{
    i2c_master_bus_handle_t i2c_bus;
    i2c_master_get_bus_handle(0,&i2c_bus);

    esp_err_t ret = pcf85063a_init(&dev, i2c_bus, PCF85063A_ADDRESS);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize PCF85063A (error: %d)", ret);
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "Set current time.");
    //ret = pcf85063a_set_time_date(&dev, Set_Time);

    return ret;
}

void get_rtc_data_to_str(pcf85063a_datetime_t *time)
{
    // Read current time from RTC
    pcf85063a_get_time_date(&dev, time);
}