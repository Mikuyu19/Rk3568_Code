#ifndef MYCODE_2048_H
#define MYCODE_2048_H

#include "lvgl/lvgl.h"

/* 创建 2048 页面。
 * back_cb 由主菜单模块传入，点击返回按钮时会回到主菜单。
 */
lv_obj_t *app_2048_create_screen(lv_event_cb_t back_cb);

#endif
