#ifndef MYCODE_ALBUM_H
#define MYCODE_ALBUM_H

#include "lvgl/lvgl.h" // 引入 LVGL 头文件

/* 创建电子相册页面。
 * back_cb 由主菜单模块传入，
 * 点击右上角“返回”按钮时会调用它回到主菜单。
 */
// 声明相册界面创建函数
lv_obj_t *album_create_screen(lv_event_cb_t back_cb);

#endif