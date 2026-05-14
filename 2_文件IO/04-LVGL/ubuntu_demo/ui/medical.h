#ifndef MEDICAL_H
#define MEDICAL_H

#include "lvgl/lvgl.h"

// 初始化并显示医疗叫号屏幕（包含网络轮询定时器）
void medical_init(void);

// 创建医疗叫号屏幕对象，back_cb 为返回按钮回调
lv_obj_t *medical_create_screen(lv_event_cb_t back_cb);

#endif
