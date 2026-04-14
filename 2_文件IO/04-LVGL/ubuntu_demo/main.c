#include "lvgl/lvgl.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "mycode/app_ui.h"

/* 读取环境变量：
 * 如果外部已经设置了变量，就使用外部值；
 * 如果没有设置，就回退到默认值。
 * 这里主要用于显示设备路径、SDL 窗口大小之类的配置。
 */
static const char *getenv_default(const char *name, const char *dflt)
{
    const char *value = getenv(name);
    return value ? value : dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    /* framebuffer 方式显示时，默认使用 /dev/fb0 */
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    /* 把 framebuffer 设备文件绑定给 LVGL 显示驱动 */
    lv_linux_fbdev_set_file(disp, device);
    /* 注册输入设备，这里默认写死成 event6。
     * 如果板子上触摸节点不是 event6，需要改这里。
     */
    lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    /* DRM 模式下使用显卡设备文件 */
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    /* SDL 模式主要用于 Ubuntu/PC 上开窗口调试 */
    const int width = atoi(getenv_default("LV_SDL_VIDEO_WIDTH", "1024"));
    const int height = atoi(getenv_default("LV_SDL_VIDEO_HEIGHT", "600"));

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

int main(void)
{
    /* LVGL 程序最标准的执行流程：
     * 1. 初始化 LVGL 内核
     * 2. 初始化显示设备和输入设备
     * 3. 创建我们自己的应用界面
     * 4. 死循环里不断处理刷新、动画、输入和事件
     */
    lv_init();
    lv_linux_disp_init();

    /* 进入 2048 模块的入口函数。
     * 后面如果你有更多页面或项目，也可以在这里切换成别的模块入口。
     */
    app_ui_create();

    while (1)
    {
        /* LVGL 的动画、定时器、界面刷新、事件处理都依赖这个函数 */
        lv_timer_handler();
        /* 稍微休眠一下，避免 CPU 一直空转 */
        usleep(5000);
    }

    return 0;
}
