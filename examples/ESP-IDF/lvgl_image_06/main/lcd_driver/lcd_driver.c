#include "lcd_driver.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/ledc.h"

#include "sdkconfig.h"
#include "esp_lcd_panel_st7789.h"

static const char *TAG = "lcd driver";

typedef struct {
    uint8_t cmd;
    const uint8_t *data;
    uint8_t data_size;
    uint16_t delay_ms;
} st7789_lcd_init_cmd_t;

/* LCD IO and panel */
esp_lcd_panel_io_handle_t lcd_io = NULL;
esp_lcd_panel_handle_t lcd_panel = NULL;

static ledc_channel_config_t ledc_channel;
static uint8_t backlight = 0;

static void Backlight_Init(void);

static void test_draw_bitmap(esp_lcd_panel_handle_t panel_handle)
{
    uint16_t row_line = ((320 / 16) << 1) >> 1;
    uint8_t byte_per_pixel = 16 / 8;
    uint8_t *color = (uint8_t *)heap_caps_calloc(1, row_line * 240 * byte_per_pixel, MALLOC_CAP_DMA);

    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < row_line * 240; i++) {
            for (int k = 0; k < byte_per_pixel; k++) {
                color[i * byte_per_pixel + k] = (SPI_SWAP_DATA_TX(BIT(j), 16) >> (k * 8)) & 0xff;
            }
        }
        esp_lcd_panel_draw_bitmap(panel_handle, 0, j * row_line, 240, (j + 1) * row_line, color);
    }
    free(color);

    vTaskDelay(pdMS_TO_TICKS(2000));
}


void draw_solid_rect(esp_lcd_panel_handle_t panel_handle, int x, int y, int w, int h, uint16_t color)
{
    // 假设使用 RGB565 格式（2字节/像素）
    size_t pixel_bytes = 2;
    size_t buf_size = w * h * pixel_bytes;
    uint8_t *buf = (uint8_t *)heap_caps_calloc(1, buf_size, MALLOC_CAP_DMA);
    if (!buf) return;

    // 填充颜色
    for (int i = 0; i < w * h; i++) {
        buf[i * 2 + 0] = color & 0xFF;
        buf[i * 2 + 1] = (color >> 8) & 0xFF;
    }

    // 绘制
    esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + w, y + h, buf);

    free(buf);

    vTaskDelay(pdMS_TO_TICKS(20000));
}


esp_err_t lcd_driver_init(void)
{
    
    Backlight_Init();

    esp_err_t ret = ESP_OK;

    /* LCD initialization */
    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_SCK,
        .mosi_io_num = LCD_DIN,
        .miso_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = EXAMPLE_LCD_H_RES * 50 * sizeof(uint16_t),
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(EXAMPLE_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC,
        .cs_gpio_num = LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLK_HZ,
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,
        .lcd_param_bits = EXAMPLE_LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)EXAMPLE_LCD_SPI_NUM, &io_config, &lcd_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = EXAMPLE_LCD_BITS_PER_PIXEL,
    };

    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel), err, TAG, "New panel failed");

    esp_lcd_panel_reset(lcd_panel);
    esp_lcd_panel_init(lcd_panel);

    esp_lcd_panel_invert_color(lcd_panel, true);
    esp_lcd_panel_disp_on_off(lcd_panel, true);

    Set_Backlight(Backlight_MAX);
    
    //test_draw_bitmap(lcd_panel);
    //draw_solid_rect(lcd_panel,40,40,200,100,0xf800);
    return ret;

err:
    if (lcd_panel) {
        esp_lcd_panel_del(lcd_panel);
    }
    if (lcd_io) {
        esp_lcd_panel_io_del(lcd_io);
    }
    spi_bus_free(EXAMPLE_LCD_SPI_NUM);
    return ret;
}


static void Backlight_Init(void)
{
    ESP_LOGI(TAG, "Turn off LCD backlight");
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_BL
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    gpio_set_level(LCD_BL,1);


    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LS_MODE,
        .timer_num = LEDC_HS_TIMER,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel.channel    = LEDC_HS_CH0_CHANNEL;
    ledc_channel.duty       = 0;
    ledc_channel.gpio_num   = LCD_BL;
    ledc_channel.speed_mode = LEDC_LS_MODE;
    ledc_channel.timer_sel  = LEDC_HS_TIMER;
    ledc_channel_config(&ledc_channel);
    ledc_fade_func_install(0);  
}

void Set_Backlight(uint8_t Light)
{   
    if(Light > Backlight_MAX) Light = Backlight_MAX;
    uint16_t Duty = LEDC_MAX_Duty-(81*(Backlight_MAX-Light));
    if(Light == 0) 
        Duty = 0;
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, Duty);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
}


uint8_t Read_Backlight_value(void)
{
    return backlight;
}