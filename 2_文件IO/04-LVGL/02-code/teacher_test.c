#include "../lvgl/lvgl.h"
#include <stdio.h>
static void test01()
{
    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 再活动屏幕上创建一个基本对象
    lv_obj_t *win = lv_obj_create(scr);

    // 设置基本对象的大小
    lv_obj_set_size(win, 100, 100);

    // 设置位置
    lv_obj_set_align(win, LV_ALIGN_RIGHT_MID);
}

static void test03()
{
    lv_obj_t *win[9];
    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建9个窗口
    for (int i = 0; i < 9; i++)
    {
        // 再活动屏幕上创建一个基本对象
        win[i] = lv_obj_create(scr);

        // 设置大小
        lv_obj_set_size(win[i], 200, 100);

        // 设置位置
        lv_obj_set_align(win[i], i + 1);

        // 为每个窗口创建子对象标签
        lv_obj_t *lab = lv_label_create(win[i]);

        // 在标签上显示数字
        lv_label_set_text_fmt(lab, "%d", i + 1);
    }
}

static void test04()
{

    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建标签
    lv_obj_t *lab = lv_label_create(scr);
    // 设置标签宽高
    lv_obj_set_size(lab, 50, 100);

    // 在标签上内容
    lv_label_set_text(lab, "adaasdahsdkasfsdhfdhff");

    // 设置文本处理模式
    lv_label_set_long_mode(lab, LV_LABEL_LONG_SCROLL_CIRCULAR);
}

static void test05()
{
    lv_obj_t *win[9];
    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建9个窗口
    for (int i = 0; i < 9; i++)
    {
        // 再活动屏幕上创建一个基本对象
        win[i] = lv_obj_create(scr);

        // 设置大小
        lv_obj_set_size(win[i], 200, 100);

        // 设置位置
        lv_obj_set_align(win[i], i + 1);

        // 为每个窗口创建子对象标签
        lv_obj_t *lab = lv_label_create(win[i]);

        // 在标签上显示数字
        lv_label_set_text_fmt(lab, "%d", i + 1);

        // 设置字体颜色
        lv_obj_set_style_text_color(lab, lv_color_hex(0xff0000), 0);

        // 修改字体大小
        lv_obj_set_style_text_font(lab, &lv_font_montserrat_30, 0);
    }
}

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
    lv_obj_set_size(lab, 100, 100);

    // 在标签上内容
    lv_label_set_text(lab, "李湘");

    // 设置字体颜色
    lv_obj_set_style_text_color(lab, lv_color_hex(0xff0000), 0);

    // 修改字体大小
    lv_obj_set_style_text_font(lab, &my_test_font_28, 0);
}

/**
 * Load a font with FreeType
 */
void test07(void)
{
    /*Create a font*/
    lv_font_t *font = lv_freetype_font_create("/mnt/hgfs/CS2612/二阶段/02-文件IO/04-LVGL/02-code/ubuntu_demo/SIMKAI.TTF",
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

static int click_count = 0;
static void test08_cb(lv_event_t *e)
{
    // 获取注册事件发生的对象
    lv_obj_t *btn = lv_event_get_target(e);
    // 获取obj上的第0个子对象
    lv_obj_t *lab = lv_obj_get_child(btn, 0);
    // 次数+1
    click_count++;

    // 更新lab
    lv_label_set_text_fmt(lab, "prv:%d", click_count);
}

/**
 * Load a font with FreeType
 */
static void test08(void)
{

    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建按钮
    lv_obj_t *btn = lv_button_create(scr);

    // 按钮设置大小
    lv_obj_set_size(btn, 100, 50);

    // 创建标签
    lv_obj_t *lab = lv_label_create(btn);

    // 在标签上内容
    lv_label_set_text_fmt(lab, "prv:%d", click_count);

    // 将按钮和函数进行绑定
    lv_obj_add_event_cb(btn, test08_cb, LV_EVENT_CLICKED, NULL);
}

static void test09_cb(lv_event_t *e)
{
    lv_point_t point;
    // 获取一个点击坐标
    lv_indev_get_point(lv_indev_active(), &point);
    LV_LOG_USER("x:%d,y:%d\n", point.x, point.y);
}

static void test09(void)
{

    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建基本对象
    lv_obj_t *win = lv_obj_create(scr);

    // 设置宽高
    lv_obj_set_size(win, 1024, 600);

    // 将win和函数进行绑定
    lv_obj_add_event_cb(win, test09_cb, LV_EVENT_CLICKED, NULL);
}

static lv_point_t start_point; // 按下
static lv_point_t end_point;   // 松开
/*
    0 点击
    1 上
    2 下
    3 左
    4 右
*/
static int get_slide(lv_point_t start_point, lv_point_t end_point)
{
    int x = end_point.x - start_point.x;
    int y = end_point.y - start_point.y;

    if (x == 0 && y == 0)
    {
        return 0;
    }
    else if (abs(x) > abs(y)) // 左和右
    {
        if (x > 0)
        {
            return 4;
        }
        else
        {
            return 3;
        }
    }
    else // 上和下
    {
        if (y > 0)
        {
            return 2;
        }
        else
        {
            return 1;
        }
    }
}
static void test10_cb(lv_event_t *e)
{

    // 获取事件编码
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED) // 按下
    {
        // 获取一个点击坐标
        lv_indev_get_point(lv_indev_active(), &start_point);
    }
    else if (code == LV_EVENT_RELEASED) // 松开
    {
        // 获取一个点击坐标
        lv_indev_get_point(lv_indev_active(), &end_point);

        // 获取滑动方向
        int r = get_slide(start_point, end_point);
        if (r == 0)
        {
            printf("click(%d,%d)\n", end_point.x, end_point.y);
        }
        else if (r == 1)
        {
            printf("up\n");
        }
        else if (r == 2)
        {
            printf("down\n");
        }
        else if (r == 3)
        {
            printf("left\n");
        }
        else if (r == 4)
        {
            printf("right\n");
        }
    }
}

static void test10(void)
{

    // 创建一个活动屏幕
    lv_obj_t *scr = lv_screen_active();

    // 创建基本对象
    lv_obj_t *win = lv_obj_create(scr);

    // 设置宽高
    lv_obj_set_size(win, 1024, 600);

    // 将win和函数进行绑定
    lv_obj_add_event_cb(win, test10_cb, LV_EVENT_ALL, NULL);
}

void test()
{
    // 想调用那个测试函数 就用哪个测试函数名字
    test10();
}
