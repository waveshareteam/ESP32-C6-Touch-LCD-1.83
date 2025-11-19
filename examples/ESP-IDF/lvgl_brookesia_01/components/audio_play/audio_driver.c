#include "audio_driver.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "bsp_board.h"
#include "driver/gpio.h"

static const char *TAG = "audio play";


static uint8_t Volume = PLAYER_VOLUME;
static esp_asp_handle_t handle = NULL;
static TaskHandle_t xHandle;

typedef enum {
    CMD_PLAY,
    CMD_STOP,
    CMD_PAUSE,
    CMD_RESUME,
    CMD_DEINIT
} player_cmd_t;

typedef struct {
    player_cmd_t cmd;
    char url[128]; // 假设最大路径长度为128
} player_queue_t;

#define QUEUE_LENGTH 5

static QueueHandle_t cmd_queue = NULL;

static void Audio_PA_EN(void)
{
    gpio_set_level(GPIO_NUM_0,1);
}
static void Audio_PA_DIS(void)
{
    gpio_set_level(GPIO_NUM_0,0);
}




void player_task(void *pvParameters)
{
    player_queue_t msg;
    while (1) {
        if (xQueueReceive(cmd_queue, &msg, portMAX_DELAY)) {
            switch (msg.cmd) {
                case CMD_PLAY:
                    printf("收到命令: 开始播放<%s>\n",msg.url);
                    Audio_PA_DIS();
                    esp_audio_simple_player_stop(handle);
                    esp_audio_simple_player_run(handle, msg.url, NULL);
                    Audio_PA_EN();
                    break;
                case CMD_STOP:
                    printf("收到命令: 停止播放\n");
                    esp_audio_simple_player_stop(handle);
                    Audio_PA_DIS();
                    break;
                case CMD_PAUSE:
                    printf("收到命令: 暂停播放\n");
                    Audio_PA_DIS();
                    esp_audio_simple_player_pause(handle);
                    break;
                case CMD_RESUME:
                    printf("收到命令: 恢复播放\n");
                    esp_audio_simple_player_resume(handle);
                    Audio_PA_EN();
                    break;
                case CMD_DEINIT:
                    gpio_set_level(GPIO_NUM_0,0);
                    ESP_ERROR_CHECK(gpio_reset_pin(GPIO_NUM_0));
                    esp_audio_simple_player_destroy(handle);
                    vQueueDelete(cmd_queue);
                    cmd_queue = NULL; 
                    vTaskDelete(NULL);
                    break;
                default:
                    printf("未知命令\n");
                    break;
            }
        }
    }
}



















static int out_data_callback(uint8_t *data, int data_size, void *ctx)
{
    lvgl_port_lock(0);
    esp_audio_play((int16_t*)data,data_size,500 / portTICK_PERIOD_MS);
    lvgl_port_unlock();
    return 0;
}

static int in_data_callback(uint8_t *data, int data_size, void *ctx)
{
    int ret = fread(data, 1, data_size, ctx);
    ESP_LOGD(TAG, "%s-%d,rd size:%d", __func__, __LINE__, ret);
    return ret;
}

static int mock_event_callback(esp_asp_event_pkt_t *event, void *ctx)
{
    if (event->type == ESP_ASP_EVENT_TYPE_MUSIC_INFO) 
    {
        esp_asp_music_info_t info = {0};
        memcpy(&info, event->payload, event->payload_size);
        ESP_LOGI(TAG, "Get info, rate:%d, channels:%d, bits:%d ,bitrate = %d", info.sample_rate, info.channels, info.bits,info.bitrate);
    } 
    else if (event->type == ESP_ASP_EVENT_TYPE_STATE) 
    {
        esp_asp_state_t st = 0;
        memcpy(&st, event->payload, event->payload_size);

        ESP_LOGI(TAG, "Get State, %d,%s", st, esp_audio_simple_player_state_to_str(st));
        if(st == ESP_ASP_STATE_FINISHED)
        {
            ESP_LOGI(TAG,"播放完成");
            Audio_PA_DIS();
        }
    }
    return 0;
}


static void pipeline_init(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);

    esp_asp_cfg_t cfg = {
        .in.cb = NULL,
        .in.user_ctx = NULL,
        .out.cb = out_data_callback,
        .out.user_ctx = NULL,
    };

    esp_gmf_err_t err = esp_audio_simple_player_new(&cfg, &handle);
    err = esp_audio_simple_player_set_event(handle, mock_event_callback, NULL);
}

static void pipeline_deinit(void)
{
    esp_audio_simple_player_destroy(handle);
}


esp_gmf_err_t Audio_Play_Music(const char* url)
{
    esp_gmf_err_t err = ESP_GMF_ERR_OK;
    player_queue_t msg;
    memset(&msg,0,sizeof(msg));
    msg.cmd = CMD_PLAY;
    memcpy(msg.url,url,strlen(url));
    xQueueSend(cmd_queue, &msg, portMAX_DELAY);
    return err;
}

esp_gmf_err_t Audio_Stop_Play(void)
{
    esp_gmf_err_t err = ESP_GMF_ERR_OK;
    player_queue_t msg;
    msg.cmd = CMD_STOP;
    xQueueSend(cmd_queue, &msg, portMAX_DELAY);
    return err;
}

esp_gmf_err_t Audio_Resume_Play(void)
{
    esp_gmf_err_t err = ESP_GMF_ERR_OK;
    player_queue_t msg;
    msg.cmd = CMD_RESUME;
    xQueueSend(cmd_queue, &msg, portMAX_DELAY);
    return err;
}

esp_gmf_err_t Audio_Pause_Play(void)
{
    esp_gmf_err_t err = ESP_GMF_ERR_OK;
    player_queue_t msg;
    msg.cmd = CMD_PAUSE;
    xQueueSend(cmd_queue, &msg, portMAX_DELAY);
    return err;
}

esp_asp_state_t Audio_Get_Current_State(void) 
{
    esp_asp_state_t state;
    esp_gmf_err_t err = esp_audio_simple_player_get_state(handle, &state);
    if (err != ESP_GMF_ERR_OK) {
        ESP_LOGE("AUDIO", "Get state failed: %d", err);
        return ESP_ASP_STATE_ERROR;
    }
    return state;
}


void Audio_Play_Init(void) 
{
    cmd_queue = xQueueCreate(QUEUE_LENGTH, sizeof(player_queue_t));
    if (cmd_queue == NULL) {
        printf("队列创建失败\n");
        return;
    }

    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << GPIO_NUM_0
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    gpio_set_level(GPIO_NUM_0,0);
    pipeline_init();
    xTaskCreate(player_task, "player_task", 2048, NULL, 5, &xHandle);
}

void Audio_Play_Deinit(void)
{
    player_queue_t msg;
    msg.cmd = CMD_DEINIT;
    xQueueSend(cmd_queue, &msg, portMAX_DELAY);
}


void Volume_Adjustment(uint8_t Vol)
{
    if(Vol > Volume_MAX )
    {
        printf("Audio : The volume value is incorrect. Please enter 0 to 21\r\n");
    }
    else  
    {
        esp_audio_set_play_vol(Vol);
        Volume = Vol;
    }
}

uint8_t get_audio_volume(void)
{
    return Volume;
}