#include "ui_font.h"

#include <limits.h>
#include <stdio.h>

/* 中文字体模块状态。
 * 这里缓存了字体路径和几个常用字号，
 * 避免每次创建标签都重复加载字体文件。
 */
typedef struct
{
    char font_path[PATH_MAX]; // 存放字库文件 STSONG.TTF 的绝对路径
    lv_font_t *font_22;       // 缓存 22 像素高的字体结构体指针
    lv_font_t *font_26;       // 缓存 26 像素高的字体结构体指针
    lv_font_t *font_30;       // 缓存 30 像素高的字体结构体指针
    lv_font_t *font_34;       // 缓存 34 像素高的字体结构体指针
    lv_font_t *font_48;       // 缓存 48 像素高的字体结构体指针
} ui_font_state_t;

// 静态全局变量管理字体
static ui_font_state_t g_ui_font;

/* 这里直接写死字体路径，避免再根据运行目录去判断。 */
static void init_font_path(void)
{
    if (g_ui_font.font_path[0] != '\0')
    {
        return;
    }
    snprintf(
        g_ui_font.font_path,
        sizeof(g_ui_font.font_path),
        "/mnt/hgfs/Rk3568_Code/2_文件IO/04-LVGL/ubuntu_demo/STSONG.TTF");
}

/* 按指定字号加载一个 FreeType 字体 */
static lv_font_t *load_font(uint32_t size)
{
    init_font_path();

    return lv_freetype_font_create(
        g_ui_font.font_path,
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
        size,
        LV_FREETYPE_FONT_STYLE_NORMAL);
}

// 获取 22 号字体
const lv_font_t *ui_font_get_22(void)
{
    if (g_ui_font.font_22 == NULL) {
        g_ui_font.font_22 = load_font(22);
    }
    return g_ui_font.font_22 ? g_ui_font.font_22 : LV_FONT_DEFAULT;
}

// 获取 26 号字体
const lv_font_t *ui_font_get_26(void)
{
    if (g_ui_font.font_26 == NULL)
    {
        g_ui_font.font_26 = load_font(26);
    }
    return g_ui_font.font_26 ? g_ui_font.font_26 : LV_FONT_DEFAULT;
}

// 获取 30 号字体
const lv_font_t *ui_font_get_30(void)
{
    if (g_ui_font.font_30 == NULL)
    {
        g_ui_font.font_30 = load_font(30);
    }
    return g_ui_font.font_30 ? g_ui_font.font_30 : LV_FONT_DEFAULT;
}

// 获取 34 号字体
const lv_font_t *ui_font_get_34(void)
{
    if (g_ui_font.font_34 == NULL)
    {
        g_ui_font.font_34 = load_font(34);
    }
    return g_ui_font.font_34 ? g_ui_font.font_34 : LV_FONT_DEFAULT;
}

const lv_font_t *ui_font_get_48(void)
{
    if (g_ui_font.font_48 == NULL)
    {
        g_ui_font.font_48 = load_font(48);
    }
    return g_ui_font.font_48 ? g_ui_font.font_48 : LV_FONT_DEFAULT;
}
