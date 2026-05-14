#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

void test01(void)
{
    // 创建一个活动窗口
    lv_obj_t *win = lv_screen_active();
    // 在活动屏幕上创建一个基本对象
    lv_obj_t *obj = lv_obj_create(win);
    // 设置对象的大小和位置
    lv_obj_set_size(obj, 1024, 600);

    // 设置对象的背景颜色和边框
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x776E65), 0);

    // 设置对象的圆角
    lv_obj_set_style_radius(obj, 10, 0);

    // 设置对象的内边距
    lv_obj_set_style_pad_all(obj, 10, 0);

    // 将对象居中显示
    lv_obj_center(obj);

    // 创建一个标签对象作为子对象
    lv_obj_t *label = lv_label_create(obj);
    // 设置标签的文本内容和样式
    lv_label_set_text(label, "Hello, LVGL!");
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0); // 白色文本
    lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);  // 设置字体
    // 将标签居中显示在父对象内
    lv_obj_center(label);
}
