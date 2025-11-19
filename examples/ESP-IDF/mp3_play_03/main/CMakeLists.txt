# 初始化源文件和包含目录
set(SOURCES
    "./main.c" 
    "./hardeware_driver/bsp_board.c"
    "./audio_play_driver/audio_driver.c"
)

set(INCLUDE_DIRS 
    "."
    "./hardeware_driver"
    "./audio_play_driver"
)


# 注册组件
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES esp_timer
    REQUIRES fatfs
    REQUIRES spi_flash
    REQUIRES esp_driver_i2s
    REQUIRES esp_driver_i2c
    REQUIRES espressif__esp_codec_dev
    REQUIRES espressif__gmf_core
    REQUIRES espressif__gmf_audio
    REQUIRES espressif__gmf_io
)