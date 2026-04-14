#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

void test02()
{
    // 1. 创建/获取一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 2. 在活动屏幕上创建基本对象
    lv_obj_t *obj = lv_obj_create(scr);

    // 3. 设置基本对象的大小 (这里设为宽 200 像素，高 150 像素)
    lv_obj_set_size(obj, 200, 150);

    // 4. 设置位置 (这里设置为相对于父屏幕居中对齐，偏移量均为 0)
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);

    // 5. 定义静态样式变量
    static lv_style_t style;

    // 6. 初始化样式
    lv_style_init(&style);

    // 7. 设置样式的背景颜色、透明度、边框颜色、边框的宽度、弧度、边框范围
    // 背景颜色
    lv_style_set_bg_color(&style, lv_color_hex(0x3498db));

    // 透明度
    lv_style_set_bg_opa(&style, LV_OPA_80);

    // 边框颜色
    lv_style_set_border_color(&style, lv_color_hex(0x2c3e50));

    // 边框的宽度
    lv_style_set_border_width(&style, 5);

    // 弧度/圆角半径
    lv_style_set_radius(&style, 15);

    // 边框范围
    lv_style_set_border_side(&style, LV_BORDER_SIDE_FULL);

    // 8. 把样式添加到窗口（对象）上
    lv_obj_add_style(obj, &style, 0);
}