#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

extern const lv_font_t my_ziti_28;
// 使用自己的字体库
static void test06()
{
    // 一定要先声明你的字体文件名字
    extern const lv_font_t my_test_font_28;
    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建标签
    lv_obj_t *lab = lv_label_create(scr);
    // 设置标签宽高
    lv_obj_set_size(lab, 200, 100);

    // 在标签上内容
    lv_label_set_text(lab, "原神牛逼");

    // 设置字体颜色
    lv_obj_set_style_text_color(lab, lv_color_hex(0xff0000), 0);

    // 修改字体大小
    lv_obj_set_style_text_font(lab, &my_ziti_28, 0);
}