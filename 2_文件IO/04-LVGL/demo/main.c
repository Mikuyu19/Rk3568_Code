#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

static const char *getenv_default(const char *name, const char *dflt)
{
    /* 读取环境变量：
     * 如果外部已经设置了变量，就用外部值；
     * 否则退回到函数给定的默认值。
     */
    return getenv(name) ?: dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    /* framebuffer 方式显示时，默认使用 /dev/fb0 */
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    /* 把 framebuffer 设备文件绑定给 LVGL 显示驱动 */
    lv_linux_fbdev_set_file(disp, device);

    /* 注册输入设备。
     * 这里当前写死成了 /dev/input/event6，通常对应触摸屏。
     * 如果你的板子触摸设备不是 event6，这里就要改。
     */
    lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    /* DRM 方式显示时，默认使用 /dev/dri/card0 */
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();

    /* 把 DRM 设备绑定给 LVGL */
    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    /* SDL 模式一般用于 PC 上调试，窗口大小可通过环境变量设置 */
    const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "1024");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "600");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

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

int main(void)
{
    /* 第一步：初始化 LVGL 核心 */
    lv_init();

    /* 第二步：初始化显示设备和输入设备 */
    lv_linux_disp_init();

    /* 这里原本是 LVGL 官方自带 demo。
     * 现在先注释掉了，后面你可以在这里替换成自己的 UI 入口函数，
     * 比如 game_box_create() 或 ui_main_create()。
     */
    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();

    /* LVGL 程序的主循环：
     * 必须不断调用 lv_timer_handler()，
     * 它会负责处理界面刷新、动画、输入事件等。
     */

    test01();
    while (1)
    {
        lv_timer_handler();
        /* 稍微休眠一下，避免 CPU 空转占满 */
        usleep(5000);
    }
    return 0;
}
