#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "pcf85063a.h"
#include "qmi8658.h"
#include "esp_lvgl_port.h"
#include "esp_wifi.h"
#include "esp_wifi.h"

//axp2101 driver bsp
esp_err_t AXP2101_driver_init(void);
void pmu_isr_handler(void);

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief I2C GPIO defineation
 * 
 */
#define I2C_NUM         (0)
#define GPIO_I2C_SCL    (GPIO_NUM_8)
#define GPIO_I2C_SDA    (GPIO_NUM_7)

/**
 * @brief lcd defineation
 * 
 */
#define LCD_SCK 1
#define LCD_DIN 2
#define LCD_CS 5
#define LCD_DC 3
#define LCD_RST 4
#define LCD_BL 6

#define TOUCH_RST -1
#define TOUCH_INT 11

#define EXAMPLE_LCD_SPI_NUM         (SPI2_HOST)
#define EXAMPLE_LCD_PIXEL_CLK_HZ    (40 * 1000 * 1000)
#define EXAMPLE_LCD_CMD_BITS        (8)
#define EXAMPLE_LCD_PARAM_BITS      (8)
#define EXAMPLE_LCD_BITS_PER_PIXEL  (16)
#define EXAMPLE_LCD_DRAW_BUFF_DOUBLE (0)
#define EXAMPLE_LCD_DRAW_BUFF_HEIGHT (30)
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

/**
 * @brief SDSPI GPIO definationv
 * 
 */
#define FUNC_SDSPI_EN       (1)
#define SDSPI_HOST          (SPI2_HOST)
#define GPIO_SDSPI_CS       (GPIO_NUM_17)
#define GPIO_SDSPI_SCLK     (GPIO_NUM_1)
#define GPIO_SDSPI_MISO     (GPIO_NUM_16)
#define GPIO_SDSPI_MOSI     (GPIO_NUM_2)

#define MOUNT_POINT "/sdcard"
#define EXAMPLE_MAX_CHAR_SIZE    64
#define MAX_FILE_NAME_SIZE 100  // Define maximum file name size
#define MAX_PATH_SIZE 512      // Define a larger size for the full path

/**
 * @brief i2s definationv
 * 
 */


#define I2S_NUM             I2S_NUM_0 
#define I2S_SAMPLE_RATE     16000 
#define I2S_CHANNEL_FORMAT  2
#define I2S_BITS_PER_CHAN   32
#define GPIO_I2S_LRCK       (GPIO_NUM_22)
#define GPIO_I2S_MCLK       (GPIO_NUM_19)
#define GPIO_I2S_SCLK       (GPIO_NUM_20)
#define GPIO_I2S_SDIN       (GPIO_NUM_21)
#define GPIO_I2S_DOUT       (GPIO_NUM_23)

//record configurations
#define RECORD_VOLUME   (50.0)

//player configurations
#define PLAYER_VOLUME   (95)

//#define FUNC_PWR_CTRL       (1)
#define GPIO_PWR_CTRL       (-1)
#define GPIO_PWR_ON_LEVEL   (1)

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)

    #define I2S_CONFIG_DEFAULT(sample_rate, channel_fmt, bits_per_chan) { \
            .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000), \
            .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(32, I2S_SLOT_MODE_STEREO), \
            .gpio_cfg = { \
                .mclk = GPIO_I2S_MCLK, \
                .bclk = GPIO_I2S_SCLK, \
                .ws   = GPIO_I2S_LRCK, \
                .dout = GPIO_I2S_DOUT, \
                .din  = GPIO_I2S_SDIN, \
            }, \
        }

#else

    #define I2S_CONFIG_DEFAULT(sample_rate, channel_fmt, bits_per_chan) { \
        .mode                   = I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX, \
        .sample_rate            = 16000, \
        .bits_per_sample        = I2S_BITS_PER_SAMPLE_32BIT, \
        .channel_format         = I2S_CHANNEL_FMT_RIGHT_LEFT, \
        .communication_format   = I2S_COMM_FORMAT_STAND_I2S, \
        .intr_alloc_flags       = ESP_INTR_FLAG_LEVEL1, \
        .dma_buf_count          = 6, \
        .dma_buf_len            = 160, \
        .use_apll               = false, \
        .tx_desc_auto_clear     = true, \
        .fixed_mclk             = 0, \
        .mclk_multiple          = I2S_MCLK_MULTIPLE_DEFAULT, \
        .bits_per_chan          = I2S_BITS_PER_CHAN_32BIT, \
    }

#endif



typedef struct {
    esp_lcd_panel_handle_t panel;      // ESP LCD panel (color) handle
    esp_lcd_panel_io_handle_t io;      // ESP LCD IO handle
    esp_lcd_touch_handle_t tp_handle;
    i2s_chan_handle_t      i2s_tx_handle;
    i2s_chan_handle_t      i2s_rx_handle;
    lv_display_t           *lvgl_disp_handle;
    lv_indev_t             *lvgl_touch_indev_handle;
    qmi8658_dev_t          *qmi8658_dev;

} bsp_handles_t;

//lcd driver bsp
esp_err_t bsp_lcd_driver_init(void);    //init
void bsp_set_backlight(uint8_t light);  //set backlight
uint8_t bsp_read_backlight_value(void); //get backlight value

//touch driver bsp
esp_err_t bsp_touch_driver_init(void);  //init lcd touch 

//i2c driver init
esp_err_t bsp_i2c_master_init(void);

//i2s driver sp
esp_err_t bsp_i2s_init(void);
esp_err_t bsp_i2s_deinit(void);

//codec driver bsp
esp_err_t bsp_codec_init(void);
esp_err_t bsp_codec_deinit(void);
esp_err_t esp_audio_set_play_vol(int volume);
esp_err_t esp_audio_get_play_vol(int *volume);
esp_err_t esp_audio_play(const int16_t* data, int length, uint32_t ticks_to_wait);
esp_err_t esp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len);
int esp_get_feed_channel(void);
char* esp_get_input_format(void);

//get bsp handle
bsp_handles_t *bsp_display_get_handles(void);
esp_err_t bsp_init(void);

//spi sd card driver bsp
esp_err_t sd_card_init(void);
uint32_t get_sdcard_total_size(void);
uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][MAX_FILE_NAME_SIZE], uint16_t maxFiles) ;

//imu driver init bsp
esp_err_t qmi8658_driver_init(void);

//rtc driver bsp
esp_err_t pcf85063a_driver_init(void);
void get_rtc_data_to_str(pcf85063a_datetime_t *time);

//lvgl display bsp
esp_err_t lvgl_driver_init(void);

//wifi port
void esp_wifi_port_init(const char *ssid, const char *pass);
void esp_wifi_port_get_ip(char *ip);
bool esp_wifi_port_scan(wifi_ap_record_t *ap_info, uint16_t *scan_number, uint16_t scan_max_num);
esp_err_t esp_wifi_port_disconnect(void);
esp_err_t esp_wifi_port_connect(void);
void esp_wifi_port_deinit(void);

//bt port
void bsp_bt_port_init(void);

#ifdef __cplusplus
}

#endif


