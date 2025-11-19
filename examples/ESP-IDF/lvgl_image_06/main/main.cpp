#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/i2c_master.h"

#include "lcd_driver.h"
#include "lvgl_driver.h"
#include "touch_driver.h"
#include "lv_demos.h"
#include <stdio.h>
static const char *TAG = "app_main";




static esp_err_t i2c_master_init(void)
{
    i2c_master_bus_handle_t i2c_bus_handle;
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = 0,
        .sda_io_num = GPIO_NUM_7,
        .scl_io_num = GPIO_NUM_8,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };

    esp_err_t ret = i2c_new_master_bus(&bus_config, &i2c_bus_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "I2C bus initialized successfully");
    return ESP_OK;  // 返回 ESP_OK 表示成功
}





// 图片对象指针（用于显示的图片控件）
static lv_obj_t *img1_obj;
static lv_obj_t *img2_obj;
static lv_obj_t *img3_obj;

// 当前显示的图片索引
static int8_t current_img = 0;

// 图片资源（LVGL图片数据结构）
LV_IMG_DECLARE(image1);
LV_IMG_DECLARE(image2);
LV_IMG_DECLARE(image3);

// 切换到指定图片
static void switch_to_image(int index) {
    // 先隐藏所有图片
    lv_obj_add_flag(img1_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
    
    // 显示指定图片
    switch(index) {
        case 0:
            lv_obj_clear_flag(img1_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 1:
            lv_obj_clear_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
            break;
        case 2:
            lv_obj_clear_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
            break;
    }
}

static void albumpage_gesture_cb(lv_event_t * e)
{

    lv_event_code_t event = lv_event_get_code(e);
    if(event == LV_EVENT_GESTURE) {
        lv_indev_wait_release(lv_indev_get_act());
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir){
            case LV_DIR_LEFT:
                current_img--;
                if(current_img<0)
                {
                    current_img = 2;
                }
                switch_to_image(current_img);
                printf("to left\n");
                break;
            case LV_DIR_RIGHT:
                current_img++;
                if(current_img>2)
                {
                    current_img = 0;
                }
                switch_to_image(current_img);
                printf("to right\n");
                break;
            default:break;
        }
    }
}


void image_slider_init(void) 
{

    lv_obj_t *ui_albumpage = lv_obj_create(lv_scr_act());
    lv_obj_clear_flag(ui_albumpage, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(ui_albumpage, LV_HOR_RES, LV_VER_RES);       // 设置屏幕大小
    lv_obj_set_style_bg_color(ui_albumpage, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_albumpage, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui_albumpage,0,LV_STATE_DEFAULT);
    lv_obj_center(ui_albumpage);

    // 创建图片对象1
    img1_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img1_obj, &image1);  // 设置图片资源
    lv_obj_center(img1_obj);
   
    lv_obj_set_align(img1_obj, LV_ALIGN_CENTER);
    lv_obj_add_flag(img1_obj, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(img1_obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(lv_scr_act(), albumpage_gesture_cb, LV_EVENT_GESTURE, NULL);


    // 创建图片对象2（初始隐藏）
    img2_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img2_obj, &image2);
    lv_obj_center(img2_obj);
    lv_obj_add_flag(img2_obj, LV_OBJ_FLAG_HIDDEN);
    
    // 创建图片对象3（初始隐藏）
    img3_obj = lv_image_create(ui_albumpage);
    lv_img_set_src(img3_obj, &image3);
    lv_obj_center(img3_obj);
    lv_obj_add_flag(img3_obj, LV_OBJ_FLAG_HIDDEN);
    
    // 初始显示第一张图片
    current_img = 0;
}

extern "C" void app_main(void)
{
    i2c_master_init();

    lcd_driver_init();
    touch_driver_init();
    lvgl_driver_init();
    
    lvgl_port_lock(0);
    image_slider_init();
    //lv_demo_stress();
    lvgl_port_unlock();

}