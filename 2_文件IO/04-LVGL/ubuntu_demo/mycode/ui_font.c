#include "ui_font.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char font_path[PATH_MAX];
    lv_font_t *font_22;
    lv_font_t *font_26;
    lv_font_t *font_30;
    lv_font_t *font_34;
} ui_font_state_t;

static ui_font_state_t g_ui_font;

static void init_font_path(void)
{
    if (g_ui_font.font_path[0] != '\0') {
        return;
    }

    const char *custom_path = getenv("LV_CH_FONT_PATH");
    if (custom_path && custom_path[0] != '\0' && access(custom_path, F_OK) == 0) {
        snprintf(g_ui_font.font_path, sizeof(g_ui_font.font_path), "%s", custom_path);
        return;
    }

    if (access("./STSONG.TTF", F_OK) == 0) {
        snprintf(g_ui_font.font_path, sizeof(g_ui_font.font_path), "./STSONG.TTF");
        return;
    }

    if (access("../STSONG.TTF", F_OK) == 0) {
        snprintf(g_ui_font.font_path, sizeof(g_ui_font.font_path), "../STSONG.TTF");
        return;
    }

    if (access("/mnt/hgfs/Rk3568_Code/2_文件IO/04-LVGL/ubuntu_demo/STSONG.TTF", F_OK) == 0) {
        snprintf(
            g_ui_font.font_path,
            sizeof(g_ui_font.font_path),
            "/mnt/hgfs/Rk3568_Code/2_文件IO/04-LVGL/ubuntu_demo/STSONG.TTF");
        return;
    }
}

static lv_font_t *load_font(uint32_t size)
{
    init_font_path();
    if (g_ui_font.font_path[0] == '\0') {
        return NULL;
    }

    return lv_freetype_font_create(
        g_ui_font.font_path,
        LV_FREETYPE_FONT_RENDER_MODE_BITMAP,
        size,
        LV_FREETYPE_FONT_STYLE_NORMAL);
}

const lv_font_t *ui_font_get_22(void)
{
    if (g_ui_font.font_22 == NULL) {
        g_ui_font.font_22 = load_font(22);
    }
    return g_ui_font.font_22 ? g_ui_font.font_22 : LV_FONT_DEFAULT;
}

const lv_font_t *ui_font_get_26(void)
{
    if (g_ui_font.font_26 == NULL) {
        g_ui_font.font_26 = load_font(26);
    }
    return g_ui_font.font_26 ? g_ui_font.font_26 : LV_FONT_DEFAULT;
}

const lv_font_t *ui_font_get_30(void)
{
    if (g_ui_font.font_30 == NULL) {
        g_ui_font.font_30 = load_font(30);
    }
    return g_ui_font.font_30 ? g_ui_font.font_30 : LV_FONT_DEFAULT;
}

const lv_font_t *ui_font_get_34(void)
{
    if (g_ui_font.font_34 == NULL) {
        g_ui_font.font_34 = load_font(34);
    }
    return g_ui_font.font_34 ? g_ui_font.font_34 : LV_FONT_DEFAULT;
}
