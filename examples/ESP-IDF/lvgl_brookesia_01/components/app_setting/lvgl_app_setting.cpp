#include "lvgl_app_setting.hpp"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "private/esp_brookesia_utils.h"

#include "app_setting_assets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_flash.h" 
#include "bsp_board.h"
#include "XPowersLib.h"
#include "wifi_scan.h"

using namespace std;
using namespace esp_brookesia::gui;


static lv_style_t style_text_muted;
static lv_obj_t * SD_Size;
static lv_obj_t * FlashSize;
static lv_obj_t * RTC_Time;
static lv_obj_t * but_bat_msg;
static lv_obj_t * but_wifi_msg;
static lv_obj_t * Backlight_slider;
static lv_timer_t * auto_step_timer = NULL;

static lv_obj_t *list_bat_msg;
static lv_obj_t *label_charging;
static lv_obj_t *label_battery_connect;
static lv_obj_t *label_vbus_in;
static lv_obj_t *label_battery_percent;
static lv_obj_t *label_battery_voltage;
static lv_obj_t *label_vbus_voltage;
static lv_obj_t *label_system_voltage;
static lv_obj_t *label_dc1_voltage;
static lv_obj_t *label_aldo1_voltage;
static lv_obj_t *label_bldo1_voltage;
static lv_obj_t *label_bldo2_voltage;
static lv_timer_t* axp_timer = NULL;
extern XPowersPMU PMU;
static void example1_increase_lvgl_tick(lv_timer_t * t);


PhoneSettingConf::PhoneSettingConf(bool use_status_bar, bool use_navigation_bar):
    ESP_Brookesia_PhoneApp("Setting", &icon_setting, true, use_status_bar, use_navigation_bar)
{
}

PhoneSettingConf::PhoneSettingConf():
    ESP_Brookesia_PhoneApp("Setting", &icon_setting, true)
{
}

PhoneSettingConf::~PhoneSettingConf()
{
    ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);

}



static void Backlight_adjustment_event_cb(lv_event_t * e) 
{
  uint8_t Backlight = lv_slider_get_value((lv_obj_t*)lv_event_get_target(e));  
  if (Backlight <= Backlight_MAX)  
  {
    lv_slider_set_value(Backlight_slider, Backlight, LV_ANIM_ON); 
    bsp_set_backlight(Backlight);
  }
  else
    printf("Backlight out of range: %d\n", Backlight);
}


static void axp2101_time_cb(lv_timer_t *timer)
{
    lv_label_set_text(label_charging, PMU.isCharging() ? "YES" : "NO");
    lv_label_set_text(label_battery_connect, PMU.isBatteryConnect() ?  "YES" : "NO");
    lv_label_set_text(label_vbus_in, PMU.isVbusIn() ?  "YES" : "NO");
    lv_label_set_text_fmt(label_battery_percent, "%d %%", PMU.getBatteryPercent());
    lv_label_set_text_fmt(label_battery_voltage, "%d mV", PMU.getBattVoltage());
    lv_label_set_text_fmt(label_vbus_voltage, "%d mV", PMU.getVbusVoltage());
    lv_label_set_text_fmt(label_system_voltage, "%d mV", PMU.getSystemVoltage());
    lv_label_set_text_fmt(label_dc1_voltage, "%d mV", PMU.getDC1Voltage());
    lv_label_set_text_fmt(label_aldo1_voltage, "%d mV", PMU.getALDO1Voltage());
    lv_label_set_text_fmt(label_bldo1_voltage, "%d mV", PMU.getBLDO1Voltage());
    lv_label_set_text_fmt(label_bldo2_voltage, "%d mV", PMU.getBLDO2Voltage());
    
}

void axp2101_tile_init(lv_obj_t *parent) 
{
    /*Create a list*/
    list_bat_msg = lv_list_create(parent);
    lv_obj_set_size(list_bat_msg, lv_pct(95), lv_pct(90));

    lv_obj_t *list_item;

    list_item = lv_list_add_btn(list_bat_msg, NULL, "isCharging");
    label_charging = lv_label_create(list_item);
    lv_label_set_text(label_charging, PMU.isCharging() ? "YES" : "NO");

    list_item = lv_list_add_btn(list_bat_msg, NULL, "isBatteryConnect");
    label_battery_connect = lv_label_create(list_item);
    lv_label_set_text(label_battery_connect, PMU.isBatteryConnect() ?  "YES" : "NO");

    list_item = lv_list_add_btn(list_bat_msg, NULL, "isVbusIn");
    label_vbus_in = lv_label_create(list_item);
    lv_label_set_text(label_vbus_in, PMU.isVbusIn() ?  "YES" : "NO");

    list_item = lv_list_add_btn(list_bat_msg, NULL, "BatteryPercent");
    label_battery_percent = lv_label_create(list_item);
    lv_label_set_text_fmt(label_battery_percent, "%d %%", PMU.getBatteryPercent());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "BatteryVoltage");
    label_battery_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_battery_voltage, "%d mV", PMU.getBattVoltage());
    
    list_item = lv_list_add_btn(list_bat_msg, NULL, "VbusVoltage");
    label_vbus_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_vbus_voltage, "%d mV", PMU.getVbusVoltage());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "SystemVoltage");
    label_system_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_system_voltage, "%d mV", PMU.getSystemVoltage());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "DC1Voltage");
    label_dc1_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_dc1_voltage, "%d mV", PMU.getDC1Voltage());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "ALDO1Voltage");
    label_aldo1_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_aldo1_voltage, "%d mV", PMU.getALDO1Voltage());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "BLDO1Voltage");
    label_bldo1_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_bldo1_voltage, "%d mV", PMU.getBLDO1Voltage());

    list_item = lv_list_add_btn(list_bat_msg, NULL, "BLDO2Voltage");
    label_bldo2_voltage = lv_label_create(list_item);
    lv_label_set_text_fmt(label_bldo2_voltage, "%d mV", PMU.getBLDO2Voltage());


    axp_timer =  lv_timer_create(axp2101_time_cb, 1000, NULL);
}

static void msg_box_button_exit_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = (lv_obj_t *) lv_event_get_user_data(e);

    // 1. 停止定时器（取消注释并完善）
    if(axp_timer != NULL) {
        lv_timer_del(axp_timer);  // 关闭定时器
        axp_timer = NULL;        // 避免野指针
    }

}

static void lv_create_msgbox(void)
{
    lv_obj_t * setting = lv_msgbox_create(NULL);
    lv_obj_set_style_clip_corner(setting, true, 0);

    lv_msgbox_add_title(setting, "AXP2101");

    /* setting fixed size */
    lv_obj_set_size(setting, 200, 250);

    /* setting's titlebar/header */
    lv_obj_t * exit_but = lv_msgbox_add_close_button(setting);
    lv_obj_add_event_cb(setting, msg_box_button_exit_event_cb, LV_EVENT_DELETE, NULL);

    /* setting's content*/
    lv_obj_t * content = lv_msgbox_get_content(setting);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_right(content, -1, LV_PART_SCROLLBAR);

    axp2101_tile_init(content);

}

// 电池按钮事件回调
static void bat_btn_event_cb(lv_event_t *e)
{
    lv_obj_t * obj = (lv_obj_t*)lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_EVENT_CLICKED)) 
    {
        lv_create_msgbox();
    }
}


static void wifi_msg_box_exit_event_cb(lv_event_t * e)
{
    lv_obj_t * mbox = (lv_obj_t *) lv_event_get_user_data(e);

    delete_lv_wifi_scan_task();

}

static void lv_create_wifi_msgbox(void)
{
    lv_obj_t * setting = lv_msgbox_create(NULL);
    lv_obj_set_style_clip_corner(setting, true, 0);

    lv_msgbox_add_title(setting, "wifi");

    /* setting fixed size */
    lv_obj_set_size(setting, 240, 284);

    /* setting's titlebar/header */
    lv_obj_t * exit_but = lv_msgbox_add_close_button(setting);
    lv_obj_add_event_cb(setting, wifi_msg_box_exit_event_cb, LV_EVENT_DELETE, NULL);

    /* setting's content*/
    lv_obj_t * content = lv_msgbox_get_content(setting);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_right(content, -1, LV_PART_SCROLLBAR);

    wifi_tile_init(content);

}

// wifi按钮事件回调
static void wifi_btn_event_cb(lv_event_t *e)
{
    lv_obj_t * obj = (lv_obj_t*)lv_event_get_target(e);
    if(lv_obj_has_state(obj, LV_EVENT_CLICKED)) 
    {
        lv_create_wifi_msgbox();
    }
}

uint32_t Flash_Searching(void)
{
    uint32_t Flash_Size;
    if(esp_flash_get_physical_size(NULL, &Flash_Size) == ESP_OK)
    {
        Flash_Size = Flash_Size / (uint32_t)(1024 * 1024);
        printf("Flash size: %ld MB\n", Flash_Size);
        return Flash_Size;
    }
    else{
        printf("Get flash size failed\n");
        return 0;
    }
    return 0;
}

extern uint32_t SDCard_Size;
bool PhoneSettingConf::run(void)
{
    ESP_BROOKESIA_LOGD("Run");

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_90);

    lv_obj_t * panel1 = lv_obj_create(lv_screen_active());
    lv_obj_set_height(panel1, LV_SIZE_CONTENT);
    lv_obj_t * panel1_title = lv_label_create(panel1);
    lv_label_set_text(panel1_title, "Onboard parameter");

    lv_obj_t * SD_label = lv_label_create(panel1);
    lv_label_set_text(SD_label, "SD Card");
    SD_Size = lv_textarea_create(panel1);
    lv_textarea_set_one_line(SD_Size, true);
    lv_textarea_set_placeholder_text(SD_Size, "SD Size");
    uint32_t sd_size = get_sdcard_total_size();
    if(sd_size == 0)
    {
        lv_textarea_set_placeholder_text(SD_Size, "No SD card is mounted");
    }
    else
    {
        char buf[100]; 
        snprintf(buf, sizeof(buf), "%ld MB\r\n", sd_size);
        lv_textarea_set_placeholder_text(SD_Size, buf);
    }
    
    //FLASH SIZE
    lv_obj_t * Flash_label = lv_label_create(panel1);
    lv_label_set_text(Flash_label, "Flash Size");
    FlashSize = lv_textarea_create(panel1);
    lv_textarea_set_one_line(FlashSize, true);
    lv_textarea_set_placeholder_text(FlashSize, "Flash Size");
    uint32_t flash_size = Flash_Searching();
    if(flash_size == 0)
    {
        lv_textarea_set_placeholder_text(FlashSize, "get flash size err");
    }
    else
    {
        char buf[100]; 
        snprintf(buf, sizeof(buf), "%ld MB\r\n", flash_size);
        lv_textarea_set_placeholder_text(FlashSize, buf);
    }


    lv_obj_t * Time_label = lv_label_create(panel1);
    lv_label_set_text(Time_label, "RTC Time");
    lv_obj_add_style(Time_label, &style_text_muted, 0);

    RTC_Time = lv_textarea_create(panel1);
    lv_textarea_set_one_line(RTC_Time, true);
    lv_textarea_set_placeholder_text(RTC_Time, "Display time");


    lv_obj_t * bat_label = lv_label_create(panel1);
    lv_label_set_text(bat_label, "battery");
    lv_obj_add_style(bat_label, &style_text_muted, 0);
    but_bat_msg = lv_button_create(panel1);
    lv_obj_set_size(but_bat_msg,200,35);
    lv_obj_add_event_cb(but_bat_msg, bat_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * bat_msg_label = lv_label_create(but_bat_msg);
    lv_obj_align(bat_msg_label,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(bat_msg_label, "bat Message");
    lv_obj_add_style(bat_msg_label, &style_text_muted, 0);

    lv_obj_t * wifi_label = lv_label_create(panel1);
    lv_label_set_text(wifi_label, "wifi");
    lv_obj_add_style(wifi_label, &style_text_muted, 0);
    but_wifi_msg = lv_button_create(panel1);
    lv_obj_set_size(but_wifi_msg,200,35);
    lv_obj_add_event_cb(but_wifi_msg, wifi_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t * wifi_msg_label = lv_label_create(but_wifi_msg);
    lv_obj_align(wifi_msg_label,LV_ALIGN_CENTER,0,0);
    lv_label_set_text(wifi_msg_label, "get wifi Message");
    lv_obj_add_style(wifi_msg_label, &style_text_muted, 0);


    lv_obj_t * Backlight_label = lv_label_create(panel1);
    lv_label_set_text(Backlight_label, "Backlight brightness");
    Backlight_slider = lv_slider_create(panel1);                                 
    lv_obj_add_flag(Backlight_slider, LV_OBJ_FLAG_CLICKABLE);    
    lv_obj_set_size(Backlight_slider, 100, 40);              
    lv_obj_set_style_radius(Backlight_slider, 3, LV_PART_KNOB);               // Adjust the value for more or less rounding                                            
    lv_obj_set_style_bg_opa(Backlight_slider, LV_OPA_TRANSP, LV_PART_KNOB);                               
    // lv_obj_set_style_pad_all(Backlight_slider, 0, LV_PART_KNOB);                                            
    lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xAAAAAA), LV_PART_KNOB);               
    lv_obj_set_style_bg_color(Backlight_slider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);             
    lv_obj_set_style_outline_width(Backlight_slider, 2, LV_PART_INDICATOR);  
    lv_obj_set_style_outline_color(Backlight_slider, lv_color_hex(0xD3D3D3), LV_PART_INDICATOR);      
    lv_slider_set_range(Backlight_slider, 5, Backlight_MAX);              
    lv_slider_set_value(Backlight_slider, DEFAULT_BACKLIGHT, LV_ANIM_ON);  
    lv_obj_add_event_cb(Backlight_slider, Backlight_adjustment_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_main_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(lv_screen_active(), grid_main_col_dsc, grid_main_row_dsc);

    static lv_coord_t grid_2_col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_FR(50), LV_GRID_FR(5), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t grid_2_row_dsc[] = {
        LV_GRID_CONTENT,  /*0: Title*/
        5,                /*1: Separator*/
        LV_GRID_CONTENT,  /*2: SD label*/
        40,               /*3: SD Size*/
        LV_GRID_CONTENT,  /*4: Flash label*/
        40,               /*5: Flash Size*/
        LV_GRID_CONTENT,  /*6: Time label*/
        40,               /*7: RTC Time*/
        LV_GRID_CONTENT,  /*8: bat label*/
        40,               /*9: bat button*/
        LV_GRID_CONTENT,  /*10: wifi label*/
        40,               /*11: wifi button*/
        LV_GRID_CONTENT,  /*12: Backlight label*/
        40,               /*13: Backlight slider*/
        LV_GRID_TEMPLATE_LAST               
    };

    // 应用网格描述（确保只设置一次，覆盖之前的 grid_1 配置）
    lv_obj_set_grid_dsc_array(panel1, grid_2_col_dsc, grid_2_row_dsc);
    lv_obj_set_grid_cell(panel1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1); // 主面板位置

    // 标题（占满列，行0）
    lv_obj_set_grid_cell(panel1_title, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_CENTER, 0, 1);

    // SD卡模块（行2-3）
    lv_obj_set_grid_cell(SD_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 2, 1);
    lv_obj_set_grid_cell(SD_Size, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 3, 1);

    // Flash模块（行4-5）
    lv_obj_set_grid_cell(Flash_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 4, 1);
    lv_obj_set_grid_cell(FlashSize, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 5, 1);

    // 时间模块（行6-7）
    lv_obj_set_grid_cell(Time_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 6, 1);
    lv_obj_set_grid_cell(RTC_Time, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 7, 1);

    // 电池模块（行8-9）
    lv_obj_set_grid_cell(bat_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 8, 1);
    lv_obj_set_grid_cell(but_bat_msg, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 9, 1);

    // WiFi模块（行10-11）
    lv_obj_set_grid_cell(wifi_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 10, 1);
    lv_obj_set_grid_cell(but_wifi_msg, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 11, 1);

    // 背光模块（行12-13）
    lv_obj_set_grid_cell(Backlight_label, LV_GRID_ALIGN_START, 0, 5, LV_GRID_ALIGN_START, 12, 1);
    lv_obj_set_grid_cell(Backlight_slider, LV_GRID_ALIGN_STRETCH, 0, 5, LV_GRID_ALIGN_CENTER, 13, 1);


    auto_step_timer = lv_timer_create(example1_increase_lvgl_tick, 1000, NULL);

    return true;
}

bool PhoneSettingConf::back(void)
{
    ESP_BROOKESIA_LOGD("Back");

    // If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
    ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");



    return true;
}



static void example1_increase_lvgl_tick(lv_timer_t * t)
{
    char buf[100];
    pcf85063a_datetime_t now_time;
    get_rtc_data_to_str(&now_time);
    snprintf(buf, sizeof(buf), "%d.%d.%d   %d:%d:%d\r\n",now_time.year,now_time.month,now_time.day,now_time.hour,now_time.min,now_time.sec);
    lv_textarea_set_placeholder_text(RTC_Time, buf);
    //printf("%s\r\n",buf);
}

bool PhoneSettingConf::close(void)
{
    ESP_BROOKESIA_LOGD("Close");
    printf(" close\r\n");
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
