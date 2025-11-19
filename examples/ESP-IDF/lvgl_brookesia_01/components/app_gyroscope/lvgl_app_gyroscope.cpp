#include "lvgl_app_gyroscope.hpp"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "private/esp_brookesia_utils.h"

#include "app_gyroscope_assets.h"
#include "bsp_board.h"


using namespace std;
using namespace esp_brookesia::gui;

static lv_timer_t * auto_step_timer = NULL;
static // 全局标签数组（8个数据项）
lv_obj_t *labels[8];  // 0:AccelX, 1:AccelY, 2:AccelZ, 3:Temp, // 4:GyroX, 5:GyroY, 6:GyroZ, 7:Time
static bsp_handles_t *handles;
static void example1_increase_lvgl_tick(lv_timer_t * t);


PhoneGyroscopeConf::PhoneGyroscopeConf(bool use_status_bar, bool use_navigation_bar):
    ESP_Brookesia_PhoneApp("Gyroscope", &icon_gyroscope, true, use_status_bar, use_navigation_bar)
{
}

PhoneGyroscopeConf::PhoneGyroscopeConf():
    ESP_Brookesia_PhoneApp("Gyroscope", &icon_gyroscope, true)
{
}

PhoneGyroscopeConf::~PhoneGyroscopeConf()
{
    ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);

}

// 初始化显示布局（左侧：加速度+温度；右侧：陀螺仪+时间戳）
void sensor_layout_init(void) {
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_size(screen, 240, 284);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x121212), 0);  // 深色背景
    lv_obj_set_scrollbar_mode(screen, LV_SCROLLBAR_MODE_OFF);      // 禁用滚动
    lv_obj_set_scroll_dir(screen, LV_DIR_NONE);

    // 1. 标题
    lv_obj_t *title = lv_label_create(screen);
    lv_label_set_text(title, "Sensor Monitor");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    // 2. 布局参数（确保内容不溢出）
    lv_coord_t left_col_x = 10;        // 左列X起点
    lv_coord_t right_col_x = 240 - 110; // 右列X起点（右侧预留10px边距）
    lv_coord_t top_y = 40;             // 数据区顶部Y起点
    lv_coord_t item_h = 38;            // 每个数据项高度
    lv_coord_t item_spacing = 3;       // 数据项间距

    // 3. 左侧数据区（加速度X/Y/Z、温度）
    // 3.1 加速度X
    labels[0] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[0], lv_color_hex(0x00FFFF), 0);  // 青色
    lv_obj_set_style_text_font(labels[0], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[0], "Accel X:\n--.- m/s²");
    lv_obj_align(labels[0], LV_ALIGN_TOP_LEFT, left_col_x, top_y);

    // 3.2 加速度Y
    labels[1] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[1], lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_text_font(labels[1], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[1], "Accel Y:\n--.- m/s²");
    lv_obj_align(labels[1], LV_ALIGN_TOP_LEFT, left_col_x, top_y + item_h + item_spacing);

    // 3.3 加速度Z
    labels[2] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[2], lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_text_font(labels[2], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[2], "Accel Z:\n--.- m/s²");
    lv_obj_align(labels[2], LV_ALIGN_TOP_LEFT, left_col_x, top_y + 2*(item_h + item_spacing));

    // 4.4 温度（左侧第四项）
    labels[3] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[3], lv_color_hex(0x00FF00), 0);  // 绿色
    lv_obj_set_style_text_font(labels[3], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[3], "Temp:\n--.- °C");
    lv_obj_align(labels[3], LV_ALIGN_TOP_LEFT, left_col_x, top_y + 3*(item_h + item_spacing));

    // 5. 右侧数据区（陀螺仪X/Y/Z、时间戳）
    // 5.1 陀螺仪X
    labels[4] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[4], lv_color_hex(0xFFAA00), 0);  // 橙色
    lv_obj_set_style_text_font(labels[4], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[4], "Gyro X:\n--.- rad/s");
    lv_obj_align(labels[4], LV_ALIGN_TOP_LEFT, right_col_x, top_y);

    // 5.2 陀螺仪Y
    labels[5] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[5], lv_color_hex(0xFFAA00), 0);
    lv_obj_set_style_text_font(labels[5], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[5], "Gyro Y:\n--.- rad/s");
    lv_obj_align(labels[5], LV_ALIGN_TOP_LEFT, right_col_x, top_y + item_h + item_spacing);

    // 5.3 陀螺仪Z
    labels[6] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[6], lv_color_hex(0xFFAA00), 0);
    lv_obj_set_style_text_font(labels[6], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[6], "Gyro Z:\n--.- rad/s");
    lv_obj_align(labels[6], LV_ALIGN_TOP_LEFT, right_col_x, top_y + 2*(item_h + item_spacing));

    // 5.4 时间戳（右侧第四项）
    labels[7] = lv_label_create(screen);
    lv_obj_set_style_text_color(labels[7], lv_color_hex(0xBB88FF), 0);  // 紫色
    lv_obj_set_style_text_font(labels[7], &lv_font_montserrat_14, 0);
    lv_label_set_text(labels[7], "Time:\n---- ms");
    lv_obj_align(labels[7], LV_ALIGN_TOP_LEFT, right_col_x, top_y + 3*(item_h + item_spacing));
}

bool PhoneGyroscopeConf::run(void)
{
    ESP_BROOKESIA_LOGD("Run");
    handles = bsp_display_get_handles();

    sensor_layout_init();
    auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 100, NULL);
    return true;
}

bool PhoneGyroscopeConf::back(void)
{
    ESP_BROOKESIA_LOGD("Back");

    // If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
    ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");



    return true;
}

static char buf[7][30];

static void example1_increase_lvgl_tick(lv_timer_t * t)
{
    qmi8658_data_t data;
    bool ready;
    esp_err_t ret = qmi8658_is_data_ready(handles->qmi8658_dev, &ready);
    if (ret == ESP_OK && ready) 
    {
        ret = qmi8658_read_sensor_data(handles->qmi8658_dev, &data);
        if (ret == ESP_OK) 
        {

            sprintf(buf[0],"%.4f",data.accelX);
            sprintf(buf[1],"%.4f",data.accelY);
            sprintf(buf[2],"%.4f",data.accelZ);
            sprintf(buf[3],"%.4f",data.temperature);
            lv_label_set_text_fmt(labels[0],"Accel X:\n %s",buf[0]);
            lv_label_set_text_fmt(labels[1],"Accel Y:\n %s",buf[1]);
            lv_label_set_text_fmt(labels[2],"Accel Z:\n %s",buf[2]);
            lv_label_set_text_fmt(labels[3],"Temp:\n %s °C",buf[3]);


            sprintf(buf[4],"%.4f",data.gyroX);
            sprintf(buf[5],"%.4f",data.gyroY);
            sprintf(buf[6],"%.4f",data.gyroZ);
            lv_label_set_text_fmt(labels[4],"Gyro X:\n %s",buf[4]);
            lv_label_set_text_fmt(labels[5],"Gyro Y:\n %s",buf[5]);
            lv_label_set_text_fmt(labels[6],"Gyro Z:\n %s",buf[6]);
            lv_label_set_text_fmt(labels[7],"Timestamp:\n %lu ",data.timestamp);

            /*ESP_LOGI("TAG", "Accel: X=%.4f m/s², Y=%.4f m/s², Z=%.4f m/s²",data.accelX, data.accelY, data.accelZ);
            ESP_LOGI("TAG", "Gyro:  X=%.4f rad/s, Y=%.4f rad/s, Z=%.4f rad/s",data.gyroX, data.gyroY, data.gyroZ);
            ESP_LOGI("TAG", "Temp:  %.2f °C, Timestamp: %lu",data.temperature, data.timestamp);*/

        } 
    } 
}

bool PhoneGyroscopeConf::close(void)
{
    ESP_BROOKESIA_LOGD("Close");

    /* Do some operations here if needed */
    ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");
    return true;
}

// bool PhoneAppComplexConf::init()
// {
//     ESP_BROOKESIA_LOGD("Init");

//     /* Do some initialization here if needed */

//     return true;
// }

// bool PhoneAppComplexConf::deinit()
// {
//     ESP_BROOKESIA_LOGD("Deinit");

//     /* Do some deinitialization here if needed */

//     return true;
// }

// bool PhoneAppComplexConf::pause()
// {
//     ESP_BROOKESIA_LOGD("Pause");

//     /* Do some operations here if needed */

//     return true;
// }

// bool PhoneAppComplexConf::resume()
// {
//     ESP_BROOKESIA_LOGD("Resume");

//     /* Do some operations here if needed */

//     return true;
// }

// bool PhoneAppComplexConf::cleanResource()
// {
//     ESP_BROOKESIA_LOGD("Clean resource");

//     /* Do some cleanup here if needed */

//     return true;
// }
