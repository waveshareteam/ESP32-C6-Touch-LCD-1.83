
file(GLOB_RECURSE SRCS 
"${CMAKE_CURRENT_SOURCE_DIR}/*.c"
"${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "include" "assets"
    REQUIRES esp-brookesia lvgl__lvgl bsp_esp32_c6_touch_lcd_1_83 XPowersLib)