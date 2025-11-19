# 初始化源文件和包含目录
set(SOURCES
    "./main.cpp" 
    "./lcd_driver/lcd_driver.c"
    "./lvgl_driver/lvgl_driver.c"
    "./touch_driver/touch_driver.c"
    "./image1"
    "./image2"
    "./image3"
)


set(INCLUDE_DIRS 
    "."
    "./lcd_driver"
    "./lvgl_driver"
    "./touch_driver"
)


# 注册组件
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES spi_flash
    REQUIRES esp_driver_i2c
)