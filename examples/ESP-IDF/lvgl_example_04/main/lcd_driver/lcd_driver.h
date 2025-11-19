#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"

#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define TOUCH_RST -1
#define TOUCH_INT 11

#define EXAMPLE_LCD_SPI_NUM         (SPI2_HOST)
#define EXAMPLE_LCD_PIXEL_CLK_HZ    (80 * 1000 * 1000)
#define EXAMPLE_LCD_CMD_BITS        (8)
#define EXAMPLE_LCD_PARAM_BITS      (8)
#define EXAMPLE_LCD_BITS_PER_PIXEL  (16)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (1)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (50)
#define EXAMPLE_LCD_BL_ON_LEVEL     (1)
#define Backlight_MAX           100   
#define DEFAULT_BACKLIGHT       90 

#define EXAMPLE_LCD_H_RES   (240)
#define EXAMPLE_LCD_V_RES   (284)

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_HS_CH0_GPIO       TEST_PIN_NUM_LCD_BK_LIGHT
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_DUTY         (4000)
#define LEDC_ResolutionRatio   LEDC_TIMER_13_BIT
#define LEDC_MAX_Duty          ((1 << LEDC_ResolutionRatio) - 1)
#ifdef __cplusplus
extern "C" {
#endif


 /* LCD IO and panel */
extern esp_lcd_panel_io_handle_t lcd_io;
extern esp_lcd_panel_handle_t lcd_panel;
//extern esp_lcd_touch_handle_t touch_handle = NULL;

esp_err_t lcd_driver_init(void);
void Set_Backlight(uint8_t Light);                   // Call this function to adjust the brightness of the backlight. The value of the parameter Light ranges from 0 to 100
uint8_t Read_Backlight_value(void);

#ifdef __cplusplus
}
#endif
