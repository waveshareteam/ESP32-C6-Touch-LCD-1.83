#pragma once


#include "bsp_board.h"

#ifdef __cplusplus
extern "C" {
#endif


void wifi_tile_init(lv_obj_t *parent);
void delete_lv_wifi_scan_task(void);

#ifdef __cplusplus
}
#endif

