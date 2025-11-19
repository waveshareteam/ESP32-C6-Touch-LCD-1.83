
file(GLOB_RECURSE SRCS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include" 
    REQUIRES esp_timer
            fatfs
            spi_flash
            esp_driver_i2s
            esp_driver_i2c
            esp_driver_gpio     
            esp_lcd     
            esp_driver_ledc        
            esp_wifi
            nvs_flash 
            bt
            espressif__esp_codec_dev
            espressif__esp_lcd_touch_cst816s
            espressif__esp_lcd_touch
            waveshare__pcf85063a
            waveshare__qmi8658
            espressif__esp_lvgl_port
            XPowersLib
)