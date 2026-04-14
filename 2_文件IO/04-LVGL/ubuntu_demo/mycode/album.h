#ifndef MYCODE_ALBUM_H
#define MYCODE_ALBUM_H

#include "lvgl/lvgl.h"

/* 创建电子相册页面。
 * back_cb 由主菜单模块传入，点击右上角 Back 按钮时返回主菜单。
 */
lv_obj_t *album_create_screen(lv_event_cb_t back_cb);

#endif
