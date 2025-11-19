#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp_board.h"
#include "esp_check.h"
#include "240_284/dark/stylesheet.hpp"
#include "esp_brookesia.hpp"
#include "private/esp_brookesia_utils.h"
#include "lvgl_music.h"

#include "lvgl_app_gyroscope.hpp"
#include "lvgl_app_setting.hpp"
#include "lvgl_app_music.hpp"
#include "lvgl_app_rec.hpp"
#include "audio_driver.h"

#include "esp_heap_caps.h"

static const char *TAG = "app_main";

static void on_clock_update_timer_cb(struct _lv_timer_t *t)
{
    time_t now;
    struct tm timeinfo;
    ESP_Brookesia_Phone *phone = (ESP_Brookesia_Phone *)t->user_data;

    time(&now);
    localtime_r(&now, &timeinfo);

    /* Since this callback is called from LVGL task, it is safe to operate LVGL */
    // Update clock on "Status Bar"
    ESP_BROOKESIA_CHECK_FALSE_EXIT(
        phone->getHome().getStatusBar()->setClock(timeinfo.tm_hour, timeinfo.tm_min),
        "Refresh status bar failed"
    );
}

extern "C" void app_main(void)
{
    bsp_init();
    bsp_handles_t *handles = bsp_display_get_handles();
    ESP_ERROR_CHECK(AXP2101_driver_init());

    

    LVGL_Search_Music();


    lvgl_port_lock(0);
    /* Create a phone object */
    ESP_Brookesia_Phone *phone = new ESP_Brookesia_Phone(handles->lvgl_disp_handle);
    ESP_BROOKESIA_CHECK_NULL_EXIT(phone, "Create phone failed");

    /* Try using a stylesheet that corresponds to the resolution */
    ESP_Brookesia_PhoneStylesheet_t *stylesheet = new ESP_Brookesia_PhoneStylesheet_t(ESP_BROOKESIA_PHONE_240_284_DARK_STYLESHEET());
    ESP_BROOKESIA_CHECK_NULL_EXIT(stylesheet, "Create stylesheet failed");

    ESP_LOGI(TAG, "Using stylesheet (%s)", stylesheet->core.name);
    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->addStylesheet(stylesheet), "Add stylesheet failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->activateStylesheet(stylesheet), "Activate stylesheet failed");
    delete stylesheet;


    /* Configure and begin the phone */
    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->setTouchDevice(handles->lvgl_touch_indev_handle), "Set touch device failed");
    phone->registerLvLockCallback((ESP_Brookesia_GUI_LockCallback_t)(lvgl_port_lock), 0);
    phone->registerLvUnlockCallback((ESP_Brookesia_GUI_UnlockCallback_t)(lvgl_port_unlock));
    ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->begin(), "Begin failed");
    // ESP_BROOKESIA_CHECK_FALSE_EXIT(phone->getCoreHome().showContainerBorder(), "Show container border failed");

    /* Install apps */
    PhoneMusicConf *app_music_conf = new PhoneMusicConf(0,0);
    ESP_BROOKESIA_CHECK_NULL_EXIT(app_music_conf, "Create app music failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(app_music_conf) >= 0), "Install app music failed");

    PhoneSettingConf *app_setting_conf = new PhoneSettingConf(0,0);
    ESP_BROOKESIA_CHECK_NULL_EXIT(app_setting_conf, "Create app setting failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(app_setting_conf) >= 0), "Install app setting failed");

    PhoneGyroscopeConf *app_gyroscope_conf = new PhoneGyroscopeConf(0,0);
    ESP_BROOKESIA_CHECK_NULL_EXIT(app_gyroscope_conf, "Create app music failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(app_gyroscope_conf) >= 0), "Install app music failed");

    PhoneRecConf *app_rec_conf = new PhoneRecConf(0,0);
    ESP_BROOKESIA_CHECK_NULL_EXIT(app_rec_conf, "Create app rec failed");
    ESP_BROOKESIA_CHECK_FALSE_EXIT((phone->installApp(app_rec_conf) >= 0), "Install app rec failed");

    /* Create a timer to update the clock */
    ESP_BROOKESIA_CHECK_NULL_EXIT(lv_timer_create(on_clock_update_timer_cb, 1000, phone), "Create clock update timer failed");

    lvgl_port_unlock();

}