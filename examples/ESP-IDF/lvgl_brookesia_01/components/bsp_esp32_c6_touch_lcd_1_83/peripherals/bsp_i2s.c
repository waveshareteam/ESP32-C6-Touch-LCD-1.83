#include "bsp_board.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"

#include "driver/i2c_master.h"

static const char *TAG = "bsp i2s";

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
static i2s_chan_handle_t            tx_handle = NULL;        // I2S tx channel handler
static i2s_chan_handle_t            rx_handle = NULL;        // I2S rx channel handler
#endif

esp_err_t bsp_i2s_init(void)
{
    int channel_format = I2S_CHANNEL_FORMAT;
    int bits_per_chan = I2S_BITS_PER_CHAN;
    esp_err_t ret_val = ESP_OK;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    i2s_slot_mode_t channel_fmt = I2S_SLOT_MODE_STEREO;
    if (channel_format == 1) {
        channel_fmt = I2S_SLOT_MODE_MONO;
    } else if (channel_format == 2) {
        channel_fmt = I2S_SLOT_MODE_STEREO;
    } else {
        ESP_LOGE(TAG, "Unable to configure channel_format %d", channel_format);
        channel_format = 1;
        channel_fmt = I2S_SLOT_MODE_MONO;
    }

    if (bits_per_chan != 16 && bits_per_chan != 32) {
        ESP_LOGE(TAG, "Unable to configure bits_per_chan %d", bits_per_chan);
        bits_per_chan = 32;
    }

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM, I2S_ROLE_MASTER);
    ret_val |= i2s_new_channel(&chan_cfg, &tx_handle, &rx_handle);
    i2s_std_config_t std_cfg = I2S_CONFIG_DEFAULT(I2S_SAMPLE_RATE, channel_fmt, bits_per_chan);
    ret_val |= i2s_channel_init_std_mode(tx_handle, &std_cfg);
    ret_val |= i2s_channel_init_std_mode(rx_handle, &std_cfg);
    ret_val |= i2s_channel_enable(tx_handle);
    ret_val |= i2s_channel_enable(rx_handle);
#else
    i2s_channel_fmt_t channel_fmt = I2S_CHANNEL_FMT_RIGHT_LEFT;
    if (channel_format == 1) {
        channel_fmt = I2S_CHANNEL_FMT_ONLY_LEFT;
    } else if (channel_format == 2) {
        channel_fmt = I2S_CHANNEL_FMT_RIGHT_LEFT;
    } else {
        ESP_LOGE(TAG, "Unable to configure channel_format %d", channel_format);
        channel_format = 1;
        channel_fmt = I2S_CHANNEL_FMT_ONLY_LEFT;
    }

    if (bits_per_chan != 16 && bits_per_chan != 32) {
        ESP_LOGE(TAG, "Unable to configure bits_per_chan %d", bits_per_chan);
        bits_per_chan = 16;
    }

    i2s_config_t i2s_config = I2S_CONFIG_DEFAULT(sample_rate, channel_fmt, bits_per_chan);

    i2s_pin_config_t pin_config = {
        .bck_io_num = GPIO_I2S_SCLK,
        .ws_io_num = GPIO_I2S_LRCK,
        .data_out_num = GPIO_I2S_DOUT,
        .data_in_num = GPIO_I2S_SDIN,
        .mck_io_num = GPIO_I2S_MCLK,
    };

    ret_val |= i2s_driver_install(i2s_num, &i2s_config, 0, NULL);
    ret_val |= i2s_set_pin(i2s_num, &pin_config);
#endif

    bsp_handles_t *handles = bsp_display_get_handles();
    handles->i2s_tx_handle = tx_handle;
    handles->i2s_rx_handle = rx_handle;
    return ret_val;
}

esp_err_t bsp_i2s_deinit(void)
{
    i2s_port_t i2s_num = I2S_NUM;
    esp_err_t ret_val = ESP_OK;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    if (i2s_num == I2S_NUM_0 && rx_handle) {
        ret_val |= i2s_channel_disable(rx_handle);
        ret_val |= i2s_del_channel(rx_handle);
        rx_handle = NULL;
    } else if (i2s_num == I2S_NUM_0  && tx_handle) {
        ret_val |= i2s_channel_disable(tx_handle);
        ret_val |= i2s_del_channel(tx_handle);
        tx_handle = NULL;
    }
#else
    ret_val |= i2s_stop(i2s_num);
    ret_val |= i2s_driver_uninstall(i2s_num);
#endif

    bsp_handles_t *handles = bsp_display_get_handles();
    handles->i2s_tx_handle = NULL;
    handles->i2s_rx_handle = NULL;

    return ret_val;
}

