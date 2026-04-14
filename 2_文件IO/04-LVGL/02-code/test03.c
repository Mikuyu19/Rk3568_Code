// 1.在开发板或者是模拟器上(活动屏幕上)创建9个窗口，让其显示在9个分区
// 2.9个窗口的大小为200*100,分别将其设置成 红橙黄绿青蓝紫黑白
// 	0xff0000
// 	0xff8000
// 	0xffff00
// 	0x00ff00
// 	0x00ffff
// 	0x0000ff
// 	0xff00ff
// 	0x000000
// 	0xffffff
// 3.在每个窗口中添加标签，标签的文本分别为1-9
// 4.在lvgl/examples/style/ 有很多样式案例，可以在主函数去调用对应的函数，然后在开发板上查看样式案例效果

#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

void test03(void)
{
    lv_obj_t *scr = lv_screen_active();

    // 设置窗口背景颜色为灰色
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xdddddd), 0);

    // 定义九种颜色的数组
    uint32_t colors[9] = {
        0xff0000, // 红色
        0xff8000, // 橙色
        0xffff00, // 黄色
        0x00ff00, // 绿色
        0x00ffff, // 青色
        0x0000ff, // 蓝色
        0xff00ff, // 紫色
        0x000000, // 黑色
        0xffffff  // 白色
    };

    // 窗口尺寸设定
    int win_width = 200;
    int win_height = 100;
    int gap = 10; // 窗口之间的间隔

    // 屏幕尺寸：1024*600
    int offset_x = (1024 - (win_width * 3 + gap * 4)) / 2; // 水平居中
    int offset_y = (600 - (win_height * 3 + gap * 4)) / 2; // 垂直居中
    for (int i = 0; i < 9; i++)
    {
        int row = i / 3; // 行号
        int col = i % 3; // 列号

        // 创建窗口对象
        lv_obj_t *win = lv_obj_create(scr);
        lv_obj_set_size(win, win_width, win_height);

        // 计算 x 和 y 坐标，使窗口均匀分布在屏幕上
        int x = offset_x + col * (win_width + gap) + gap;
        int y = offset_y + row * (win_height + gap) + gap;
        lv_obj_set_pos(win, x, y);

        // 设置背景颜色
        lv_obj_set_style_bg_color(win, lv_color_hex(colors[i]), 0);
        // 去除边框
        lv_obj_set_style_border_width(win, 0, 0);
        // 去除圆角显示
        lv_obj_set_style_radius(win, 0, 0);

        // 在窗口中添加标签
        lv_obj_t *label = lv_label_create(win);
        lv_label_set_text_fmt(label, "%d", i + 1);  // 设置标签文本为1-9
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // 标签居中显示
    }
}

// void ui_exercise_spiral_align(void)
// {
//     // 获取当前活动屏幕
//     lv_obj_t *scr = lv_screen_active();
//     lv_obj_set_style_bg_color(scr, lv_color_hex(0xdddddd), 0);

//     // 1. 创建一个 300x300 的容器，把它居中放在屏幕上
//     // 这样做的好处是，9个方块是对齐在容器的边缘，而不是大屏幕的边缘
//     lv_obj_t *container = lv_obj_create(scr);
//     lv_obj_set_size(container, 300, 300);
//     lv_obj_align(container, LV_ALIGN_CENTER, 0, 0);
//     // 去除容器的内边距，让里面的方块完全贴边
//     lv_obj_set_style_pad_all(container, 0, 0);

//     // 2. 将你想要的 1~9 顺序，映射到 LVGL 的 9 种对齐宏上
//     // 顺序：1(左上), 2(中上), 3(右上), 4(右中), 5(右下), 6(中下), 7(左下), 8(左中), 9(正中)
//     lv_align_t spiral_aligns[9] = {
//         LV_ALIGN_TOP_LEFT,     // 数字 1
//         LV_ALIGN_TOP_MID,      // 数字 2
//         LV_ALIGN_TOP_RIGHT,    // 数字 3
//         LV_ALIGN_RIGHT_MID,    // 数字 4
//         LV_ALIGN_BOTTOM_RIGHT, // 数字 5
//         LV_ALIGN_BOTTOM_MID,   // 数字 6
//         LV_ALIGN_BOTTOM_LEFT,  // 数字 7
//         LV_ALIGN_LEFT_MID,     // 数字 8
//         LV_ALIGN_CENTER        // 数字 9
//     };

//     // 3种颜色区分一下，好看点
//     uint32_t colors[9] = {
//         0xff0000, 0xff8000, 0xffff00,
//         0x00ff00, 0x00ffff, 0x0000ff,
//         0xff00ff, 0x000000, 0xffffff};

//     // 3. 循环生成 9 个方块
//     for (int i = 0; i < 9; i++)
//     {
//         // 创建方块并设置大小 (容器是300，分成3份，每个方块设为100刚好填满)
//         lv_obj_t *box = lv_obj_create(container);
//         lv_obj_set_size(box, 100, 100);
//         lv_obj_set_style_radius(box, 0, 0); // 去掉圆角，拼得更严丝合缝
//         lv_obj_set_style_bg_color(box, lv_color_hex(colors[i]), 0);

//         // 【核心代码】：直接应用对齐数组！不需要算坐标！
//         lv_obj_align(box, spiral_aligns[i], 0, 0);

//         // 创建标签，显示数字 (i+1 就是 1~9)
//         lv_obj_t *label = lv_label_create(box);
//         lv_label_set_text_fmt(label, "%d", i + 1);
//         lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // 标签在方块内居中

//         // 如果是黑底，字变成白色
//         if (i == 7 || i == 5)
//         {
//             lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
//         }
//         else
//         {
//             lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
//         }
//     }
// }