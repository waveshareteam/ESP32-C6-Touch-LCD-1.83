#include "bsp_board.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/i2c_master.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "esp_codec_dev_os.h"

static const char *TAG = "bsp codec";

#define ADC_I2S_CHANNEL 4
static int s_play_sample_rate = 16000;
static int s_play_channel_format = 2;
static int s_bits_per_chan = 32;

//es7210
static audio_codec_data_if_t *record_data_if  = NULL;
static audio_codec_ctrl_if_t *record_ctrl_if  = NULL;
static audio_codec_if_t *record_codec_if      = NULL;
static esp_codec_dev_handle_t record_dev      = NULL;

//es8311
static audio_codec_data_if_t *play_data_if    = NULL;
static audio_codec_ctrl_if_t *play_ctrl_if    = NULL;
static audio_codec_gpio_if_t *play_gpio_if    = NULL;
static audio_codec_if_t *play_codec_if        = NULL;
static esp_codec_dev_handle_t play_dev        = NULL;

static i2s_chan_handle_t            tx_handle = NULL;        // I2S tx channel handler
static i2s_chan_handle_t            rx_handle = NULL; 
static i2c_master_bus_handle_t      i2c_bus= NULL;

esp_err_t bsp_codec_adc_init(int sample_rate)
{
    esp_err_t ret_val = ESP_OK;

    // Do initialize of related interface: data_if, ctrl_if and gpio_if
    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        .rx_handle = rx_handle,
        .tx_handle = NULL,
#endif
    };
    record_data_if = audio_codec_new_i2s_data(&i2s_cfg);

    audio_codec_i2c_cfg_t i2c_cfg = {.addr = ES7210_CODEC_DEFAULT_ADDR,.bus_handle = i2c_bus};
    record_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    // New input codec interface
    es7210_codec_cfg_t es7210_cfg = {
        .ctrl_if = record_ctrl_if,
        .mic_selected = ES7210_SEL_MIC1 | ES7210_SEL_MIC2 | ES7210_SEL_MIC3 | ES7210_SEL_MIC4,
    };  
    record_codec_if = es7210_codec_new(&es7210_cfg);
    // New input codec device
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = record_codec_if,
        .data_if = record_data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_IN,
    };
    record_dev = esp_codec_dev_new(&dev_cfg);

    esp_codec_dev_sample_info_t fs = {
        .sample_rate = 16000,
        .channel = 2,
        .bits_per_sample = 32,
    };
    esp_codec_dev_open(record_dev, &fs);
    //esp_codec_dev_set_in_gain(record_dev, RECORD_VOLUME);
    esp_codec_dev_set_in_channel_gain(record_dev, ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0), RECORD_VOLUME);
    esp_codec_dev_set_in_channel_gain(record_dev, ESP_CODEC_DEV_MAKE_CHANNEL_MASK(1), RECORD_VOLUME);
    esp_codec_dev_set_in_channel_gain(record_dev, ESP_CODEC_DEV_MAKE_CHANNEL_MASK(2), RECORD_VOLUME);
    esp_codec_dev_set_in_channel_gain(record_dev, ESP_CODEC_DEV_MAKE_CHANNEL_MASK(3), RECORD_VOLUME);

    return ret_val;
}

esp_err_t bsp_codec_dac_init(int sample_rate, int channel_format, int bits_per_chan)
{
    esp_err_t ret_val = ESP_OK;

    // Do initialize of related interface: data_if, ctrl_if and gpio_if
    audio_codec_i2s_cfg_t i2s_cfg = {
        .port = I2S_NUM_0,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
        .rx_handle = NULL,
        .tx_handle = tx_handle,
#endif
    };
    play_data_if = audio_codec_new_i2s_data(&i2s_cfg);

    audio_codec_i2c_cfg_t i2c_cfg = {.addr = ES8311_CODEC_DEFAULT_ADDR,.bus_handle = i2c_bus};
    play_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    play_gpio_if = audio_codec_new_gpio();
    // New output codec interface
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_DAC,
        .ctrl_if = play_ctrl_if,
        .gpio_if = play_gpio_if,
        .pa_pin = GPIO_PWR_CTRL,
        .use_mclk = false,
    };
    play_codec_if = es8311_codec_new(&es8311_cfg);
    // New output codec device
    esp_codec_dev_cfg_t dev_cfg = {
        .codec_if = play_codec_if,
        .data_if = play_data_if,
        .dev_type = ESP_CODEC_DEV_TYPE_OUT,
    };
    play_dev = esp_codec_dev_new(&dev_cfg);

    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = bits_per_chan,
        .sample_rate = sample_rate,
        .channel = channel_format,
    };
    esp_codec_dev_set_out_vol(play_dev, PLAYER_VOLUME);
    esp_codec_dev_open(play_dev, &fs);

    return ret_val;
}

static esp_err_t bsp_codec_adc_deinit()
{
    esp_err_t ret_val = ESP_OK;

    if (record_dev) {
        esp_codec_dev_close(record_dev);
        esp_codec_dev_delete(record_dev);
        record_dev = NULL;
    }

    // Delete codec interface
    if (record_codec_if) {
        audio_codec_delete_codec_if(record_codec_if);
        record_codec_if = NULL;
    }
    
    // Delete codec control interface
    if (record_ctrl_if) {
        audio_codec_delete_ctrl_if(record_ctrl_if);
        record_ctrl_if = NULL;
    }
    
    // Delete codec data interface
    if (record_data_if) {
        audio_codec_delete_data_if(record_data_if);
        record_data_if = NULL;
    }

    return ret_val;
}

static esp_err_t bsp_codec_dac_deinit()
{
    esp_err_t ret_val = ESP_OK;

    if (play_dev) {
        esp_codec_dev_close(play_dev);
        esp_codec_dev_delete(play_dev);
        play_dev = NULL;
    }

    // Delete codec interface
    if (play_codec_if) {
        audio_codec_delete_codec_if(play_codec_if);
        play_codec_if = NULL;
    }
    
    // Delete codec control interface
    if (play_ctrl_if) {
        audio_codec_delete_ctrl_if(play_ctrl_if);
        play_ctrl_if = NULL;
    }
    
    if (play_gpio_if) {
        audio_codec_delete_gpio_if(play_gpio_if);
        play_gpio_if = NULL;
    }
    
    // Delete codec data interface
    if (play_data_if) {
        audio_codec_delete_data_if(play_data_if);
        play_data_if = NULL;
    }

    return ret_val;
}

esp_err_t esp_audio_set_play_vol(int volume)
{
    if (!play_dev) {
        ESP_LOGE(TAG, "DAC codec init fail");
        return ESP_FAIL;
    }
    esp_codec_dev_set_out_vol(play_dev, volume);
    return ESP_OK;
}

esp_err_t esp_audio_get_play_vol(int *volume)
{
    if (!play_dev) {
        ESP_LOGE(TAG, "DAC codec init fail");
        return ESP_FAIL;
    }
    esp_codec_dev_get_out_vol(play_dev, volume);
    return ESP_OK;
}

esp_err_t bsp_codec_init(void)
{
    esp_err_t ret_val = ESP_OK;

    i2c_master_get_bus_handle(I2C_NUM,&i2c_bus);

    bsp_handles_t *handles = bsp_display_get_handles();
    tx_handle = handles->i2s_tx_handle;
    rx_handle = handles->i2s_rx_handle;
    ret_val |= bsp_codec_adc_init(I2S_SAMPLE_RATE);
    ret_val |= bsp_codec_dac_init(I2S_SAMPLE_RATE, I2S_CHANNEL_FORMAT, I2S_BITS_PER_CHAN);

    return ret_val;
}

esp_err_t bsp_codec_deinit(void)
{
    esp_err_t ret_val = ESP_OK;
    tx_handle = NULL;
    rx_handle = NULL;
    ret_val |= bsp_codec_adc_deinit();
    ret_val |= bsp_codec_dac_deinit();
    return ret_val;
}

esp_err_t esp_audio_play(const int16_t* data, int length, uint32_t ticks_to_wait)
{
    size_t bytes_write = 0;
    esp_err_t ret = ESP_OK;
    if (!play_dev) {
        return ESP_FAIL;
    }

    int out_length= length;
    int audio_time = 1;
    audio_time *= (16000 / s_play_sample_rate);
    audio_time *= (2 / s_play_channel_format);

    int *data_out = NULL;
    if (s_bits_per_chan != 32) {
        out_length = length * 2;
        data_out = malloc(out_length);
        for (int i = 0; i < length / sizeof(int16_t); i++) {
            int ret = data[i];
            data_out[i] = ret << 16;
        }
    }

    int *data_out_1 = NULL;
    if (s_play_channel_format != 2 || s_play_sample_rate != 16000) {
        out_length *= audio_time;
        data_out_1 = malloc(out_length);
        int *tmp_data = NULL;
        if (data_out != NULL) {
            tmp_data = data_out;
        } else {
            tmp_data = (int *)data;
        }

        for (int i = 0; i < out_length / (audio_time * sizeof(int)); i++) {
            for (int j = 0; j < audio_time; j++) {
                data_out_1[audio_time * i + j] = tmp_data[i];
            }
        }
        if (data_out != NULL) {
            free(data_out);
            data_out = NULL;
        }
    }

    if (data_out != NULL) {
        ret = esp_codec_dev_write(play_dev, (void *)data_out, out_length);
        free(data_out);
    } else if (data_out_1 != NULL) {
        ret = esp_codec_dev_write(play_dev, (void *)data_out_1, out_length);
        free(data_out_1);
    } else {
        ret = esp_codec_dev_write(play_dev, (void *)data, length);
    }

    return ret;
}

esp_err_t esp_get_feed_data(bool is_get_raw_channel, int16_t *buffer, int buffer_len)
{
    esp_err_t ret = ESP_OK;
    size_t bytes_read;
    int audio_chunksize = buffer_len / (sizeof(int16_t) * ADC_I2S_CHANNEL);

    ret = esp_codec_dev_read(record_dev, (void *)buffer, buffer_len);
    if (!is_get_raw_channel) {
        for (int i = 0; i < audio_chunksize; i++) {
            int16_t ref = buffer[4 * i + 0];
            buffer[3 * i + 0] = buffer[4 * i + 1];
            buffer[3 * i + 1] = buffer[4 * i + 3];
            buffer[3 * i + 2] = ref;
        }
    }

    return ret;
}

int esp_get_feed_channel(void)
{
    return ADC_I2S_CHANNEL;
}

char* esp_get_input_format(void)
{
    return "RMNM";
}