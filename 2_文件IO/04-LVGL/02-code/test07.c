#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

void test07(void)
{
    /*Create a font*/
    lv_font_t *font = lv_freetype_font_create("/mnt/hgfs/Rk3568_Code/2_文件IO/04-LVGL/ubuntu_demo/STSONG.TTF",
                                              LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
                                              32,
                                              LV_FREETYPE_FONT_STYLE_NORMAL);

    if (!font)
    {
        LV_LOG_ERROR("freetype font create failed.");
        return;
    }

    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建标签
    lv_obj_t *lab = lv_label_create(scr);
    // 设置标签宽高
    lv_obj_set_size(lab, 400, 400);

    // 在标签上内容
    lv_label_set_text(lab, "你是谁哈哈哈");

    // 设置字体颜色
    lv_obj_set_style_text_color(lab, lv_color_hex(0xff0000), 0);

    // 修改字体大小
    lv_obj_set_style_text_font(lab, font, 0);
}
