#include "lvgl_app_music.hpp"
#include "lvgl.h"
#include "esp_brookesia.hpp"
#include "private/esp_brookesia_utils.h"

#include "app_music_assets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//#include "lv_demos.h"
#include "bsp_board.h"
#include "lvgl_music.h"
#include "audio_driver.h"

using namespace std;
using namespace esp_brookesia::gui;


PhoneMusicConf::PhoneMusicConf(bool use_status_bar, bool use_navigation_bar):
    ESP_Brookesia_PhoneApp("Music", &icon_music, true, use_status_bar, use_navigation_bar)
{
}

PhoneMusicConf::PhoneMusicConf():
    ESP_Brookesia_PhoneApp("Music", &icon_music, true)
{
}

PhoneMusicConf::~PhoneMusicConf()
{
    ESP_BROOKESIA_LOGD("Destroy(@0x%p)", this);

}


bool PhoneMusicConf::run(void)
{
    ESP_BROOKESIA_LOGD("Run");
    Audio_Play_Init();
    lvgl_music_create(lv_screen_active());
    //lv_demo_music();
    //lv_example_1();
    return true;
}

bool PhoneMusicConf::back(void)
{
    ESP_BROOKESIA_LOGD("Back");

    // If the app needs to exit, call notifyCoreClosed() to notify the core to close the app
    ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");



    return true;
}



static void example1_increase_lvgl_tick(lv_timer_t * t)
{

}

bool PhoneMusicConf::close(void)
{
    ESP_BROOKESIA_LOGD("Close");

    /* Do some operations here if needed */
    ESP_BROOKESIA_CHECK_FALSE_RETURN(notifyCoreClosed(), false, "Notify core closed failed");

    //Audio_Stop_Play();
    Audio_Play_Deinit();
    
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
