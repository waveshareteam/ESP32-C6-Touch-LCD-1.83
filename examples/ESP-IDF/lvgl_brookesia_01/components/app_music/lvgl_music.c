/**
 * @file lv_demo_music.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl_music.h"
#include "bsp_board.h"

static char SD_Name[20][100]; 
uint16_t file_count; 

void LVGL_Search_Music(void) 
{        
    file_count = Folder_retrieval("/sdcard",".mp3",SD_Name,100);
    printf("file_count=%d\r\n",file_count);                                                        
}


#include "lvgl_music_main.h"
#include "lvgl_music_list.h"
#include "string.h"
#include "esp_log.h"



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_DEMO_MUSIC_AUTO_PLAY
    static void auto_step_cb(lv_timer_t * timer);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t * ctrl;
static lv_obj_t * list;

void LVGL_Search_Music(void);

static const char * title_list[] = {
    "Waiting for true love",
    "Need a Better Future",
    "Vibrations",
    "Why now?",
    "Never Look Back",
    "It happened Yesterday",
    "Feeling so High",
    "Go Deeper",
    "Find You There",
    "Until the End",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
};

static const char * artist_list[] = {
    "The John Smith Band",
    "My True Name",
    "Robotics",
    "John Smith",
    "My True Name",
    "Robotics",
    "Robotics",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
    "Unknown artist",
};

static const char * genre_list[] = {
    "Rock - 1997",
    "Drum'n bass - 2016",
    "Psy trance - 2020",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
    "Metal - 2015",
};

static const uint32_t time_list[] = {
    1 * 60 + 14,
    2 * 60 + 26,
    1 * 60 + 54,
    2 * 60 + 24,
    2 * 60 + 37,
    3 * 60 + 33,
    1 * 60 + 56,
    3 * 60 + 31,
    2 * 60 + 20,
    2 * 60 + 19,
    2 * 60 + 20,
    2 * 60 + 19,
    2 * 60 + 20,
    2 * 60 + 19,
};

 
void lv_no_find_mp3_file_note(lv_obj_t * parent)
{
    lv_obj_t * label1 = lv_label_create(parent);
    lv_label_set_text(label1, "No MP3 files were found.");
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);

}

void lvgl_music_create(lv_obj_t * parent)
{

    if(file_count > 0)
    {
        lv_obj_set_style_bg_color(parent, lv_color_hex(0x343247), 0);

        list = lv_demo_music_list_create(parent);
        ctrl = lv_demo_music_main_create(parent);
    }
    else
    {
        lv_no_find_mp3_file_note(parent);
    }
    

}

const char * lvgl_music_get_title(uint32_t track_id)
{
    if(track_id >= file_count) return NULL;
    return SD_Name[track_id];
}

/*const char * lvgl_music_get_artist(uint32_t track_id)
{
    if(track_id >= sizeof(artist_list) / sizeof(artist_list[0])) return NULL;
    return artist_list[track_id];
}

const char * lvgl_music_get_genre(uint32_t track_id)
{
    if(track_id >= sizeof(genre_list) / sizeof(genre_list[0])) return NULL;
    return genre_list[track_id];
}*/

uint32_t lvgl_music_get_track_length(uint32_t track_id)
{
    if(track_id >= file_count) return 0;
    return time_list[track_id];
}
