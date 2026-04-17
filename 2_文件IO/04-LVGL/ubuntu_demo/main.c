#include "lvgl/lvgl.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mycode/app_ui.h"

/* 读取环境变量。
 * 如果外部设置了变量，就使用外部值；
 * 否则回退到默认值。
 */
static const char *getenv_default(const char *name, const char *dflt)
{
    const char *value = getenv(name);
    return value ? value : dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    /* framebuffer 模式默认使用 /dev/fb0 */
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    /* 把 framebuffer 设备交给 LVGL */
    lv_linux_fbdev_set_file(disp, device);

    /* 注册触摸输入设备。
     * 如果你的板子触摸不是 event6，这里需要改成实际节点。
     */
    lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    /* DRM 模式使用显卡设备 */
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    /* SDL 模式主要用于 Ubuntu/PC 调试 */
    const int width = atoi(getenv_default("LV_SDL_VIDEO_WIDTH", "1024"));
    const int height = atoi(getenv_default("LV_SDL_VIDEO_HEIGHT", "600"));

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

int main(void)
{

    lv_init();
    lv_linux_disp_init();
    app_ui_create();

    while (1) {
        
        lv_timer_handler();
        
        usleep(5000);
    }

    return 0;
}
