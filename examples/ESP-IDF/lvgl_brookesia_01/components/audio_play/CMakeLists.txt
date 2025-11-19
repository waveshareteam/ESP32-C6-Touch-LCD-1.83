
file(GLOB_RECURSE SRCS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.c")

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include" 
    REQUIRES    bsp_esp32_c6_touch_lcd_1_83      
                espressif__gmf_core
                espressif__gmf_audio
                espressif__gmf_io 
                espressif__esp_audio_simple_player
            )