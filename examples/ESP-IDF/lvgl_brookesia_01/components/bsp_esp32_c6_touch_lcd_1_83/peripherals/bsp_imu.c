#include "bsp_board.h"
#include "esp_check.h"
#include "esp_log.h"
#include "driver/i2c_master.h"


static const char *TAG = "bsp imu";

static qmi8658_dev_t qmi8658_dev;

esp_err_t qmi8658_driver_init(void)
{
    i2c_master_bus_handle_t i2c_bus;
    i2c_master_get_bus_handle(0,&i2c_bus);

    ESP_LOGI(TAG, "Initializing QMI8658...");
    esp_err_t ret = qmi8658_init(&qmi8658_dev, i2c_bus, QMI8658_ADDRESS_HIGH);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize QMI8658 (error: %d)", ret);
        vTaskDelete(NULL);
    }

    qmi8658_set_accel_range(&qmi8658_dev, QMI8658_ACCEL_RANGE_8G);
    qmi8658_set_accel_odr(&qmi8658_dev, QMI8658_ACCEL_ODR_1000HZ);
    qmi8658_set_gyro_range(&qmi8658_dev, QMI8658_GYRO_RANGE_512DPS);
    qmi8658_set_gyro_odr(&qmi8658_dev, QMI8658_GYRO_ODR_1000HZ);
    
    qmi8658_set_accel_unit_mps2(&qmi8658_dev, true);
    qmi8658_set_gyro_unit_rads(&qmi8658_dev, true);
    
    qmi8658_set_display_precision(&qmi8658_dev, 4);

    bsp_handles_t *handles = bsp_display_get_handles();
    handles->qmi8658_dev = &qmi8658_dev;
    return ret;
}


// static void qmi8658_test_task(void *arg) {
//     i2c_master_bus_handle_t bus_handle = (i2c_master_bus_handle_t)arg;
//     qmi8658_dev_t dev;
//     qmi8658_data_t data;
    
//     ESP_LOGI(TAG, "Initializing QMI8658...");
//     esp_err_t ret = qmi8658_init(&dev, bus_handle, QMI8658_ADDRESS_HIGH);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to initialize QMI8658 (error: %d)", ret);
//         vTaskDelete(NULL);
//     }

//     qmi8658_set_accel_range(&dev, QMI8658_ACCEL_RANGE_8G);
//     qmi8658_set_accel_odr(&dev, QMI8658_ACCEL_ODR_1000HZ);
//     qmi8658_set_gyro_range(&dev, QMI8658_GYRO_RANGE_512DPS);
//     qmi8658_set_gyro_odr(&dev, QMI8658_GYRO_ODR_1000HZ);
    
//     qmi8658_set_accel_unit_mps2(&dev, true);
//     qmi8658_set_gyro_unit_rads(&dev, true);
    
//     qmi8658_set_display_precision(&dev, 4);

//     while (1) {
//         bool ready;
//         ret = qmi8658_is_data_ready(&dev, &ready);
//         if (ret == ESP_OK && ready) {
//             ret = qmi8658_read_sensor_data(&dev, &data);
//             if (ret == ESP_OK) {
//                 ESP_LOGI(TAG, "Accel: X=%.4f m/s², Y=%.4f m/s², Z=%.4f m/s²",
//                          data.accelX, data.accelY, data.accelZ);
//                 ESP_LOGI(TAG, "Gyro:  X=%.4f rad/s, Y=%.4f rad/s, Z=%.4f rad/s",
//                          data.gyroX, data.gyroY, data.gyroZ);
//                 ESP_LOGI(TAG, "Temp:  %.2f °C, Timestamp: %lu",
//                          data.temperature, data.timestamp);
//                 ESP_LOGI(TAG, "----------------------------------------");
//             } else {
//                 ESP_LOGE(TAG, "Failed to read sensor data (error: %d)", ret);
//             }
//         } else {
//             ESP_LOGE(TAG, "Data not ready or error reading status (error: %d)", ret);
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }