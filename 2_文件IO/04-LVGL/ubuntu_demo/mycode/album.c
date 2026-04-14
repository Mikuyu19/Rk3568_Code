#include "album.h"
#include "ui_font.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ALBUM_IMAGE_COUNT 3

static const char *g_album_files[ALBUM_IMAGE_COUNT] = {
    "a.bmp",
    "b.bmp",
    "c.bmp"
};

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *image;
    lv_obj_t *tip_label;
    int current_index;
    char pic_dir[PATH_MAX];
    char image_path[PATH_MAX];
} album_ui_t;

static album_ui_t g_album;

static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 96, 42);
    lv_obj_set_style_radius(button, 14, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4C7CF0), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, ui_font_get_22(), 0);
    lv_obj_center(label);

    return button;
}

static void init_pic_dir(void)
{
    if (g_album.pic_dir[0] != '\0') {
        return;
    }

    const char *custom_dir = getenv("LV_ALBUM_PIC_DIR");
    if (custom_dir && custom_dir[0] != '\0') {
        snprintf(g_album.pic_dir, sizeof(g_album.pic_dir), "%s", custom_dir);
        return;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(g_album.pic_dir, sizeof(g_album.pic_dir), "A:%s/pic", cwd);
        return;
    }

    snprintf(g_album.pic_dir, sizeof(g_album.pic_dir), "A:/tmp/pic");
}

static void refresh_album_image(void)
{
    char *dst = g_album.image_path;
    size_t dst_size = sizeof(g_album.image_path);
    size_t dir_len = strlen(g_album.pic_dir);

    /* 预留足够空间给 "/a.bmp" 这种后缀，避免路径截断警告 */
    if (dir_len > dst_size - 16) {
        dir_len = dst_size - 16;
    }

    memcpy(dst, g_album.pic_dir, dir_len);
    dst[dir_len] = '\0';

    snprintf(dst + dir_len, dst_size - dir_len, "/%s", g_album_files[g_album.current_index]);

    lv_image_set_src(g_album.image, g_album.image_path);
    lv_label_set_text_fmt(
        g_album.tip_label,
        "第 %d / %d 张",
        g_album.current_index + 1,
        ALBUM_IMAGE_COUNT);
}

static void on_prev_image(lv_event_t *e)
{
    LV_UNUSED(e);

    g_album.current_index--;
    if (g_album.current_index < 0) {
        g_album.current_index = ALBUM_IMAGE_COUNT - 1;
    }

    refresh_album_image();
}

static void on_next_image(lv_event_t *e)
{
    LV_UNUSED(e);

    g_album.current_index++;
    if (g_album.current_index >= ALBUM_IMAGE_COUNT) {
        g_album.current_index = 0;
    }

    refresh_album_image();
}

static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF5F1EB), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *album_create_screen(lv_event_cb_t back_cb)
{
    memset(&g_album, 0, sizeof(g_album));
    init_pic_dir();

    g_album.screen = lv_obj_create(NULL);
    style_base_screen(g_album.screen);

    lv_obj_t *title = lv_label_create(g_album.screen);
    lv_label_set_text(title, "电子相册");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5D5246), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    /* 返回按钮放在右上角 */
    lv_obj_t *back_button = create_text_button(g_album.screen, "返回", back_cb);
    lv_obj_align(back_button, LV_ALIGN_TOP_RIGHT, -20, 16);

    lv_obj_t *frame = lv_obj_create(g_album.screen);
    lv_obj_set_size(frame, 620, 380);
    lv_obj_align(frame, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_radius(frame, 18, 0);
    lv_obj_set_style_bg_color(frame, lv_color_hex(0xDDD4C7), 0);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(frame, 0, 0);
    lv_obj_set_style_pad_all(frame, 12, 0);
    lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);

    g_album.image = lv_image_create(frame);
    lv_obj_center(g_album.image);

    /* 左边按钮切换上一张 */
    lv_obj_t *prev_button = create_text_button(g_album.screen, "上一张", on_prev_image);
    lv_obj_align_to(prev_button, frame, LV_ALIGN_OUT_LEFT_MID, -24, 0);

    /* 右边按钮切换下一张 */
    lv_obj_t *next_button = create_text_button(g_album.screen, "下一张", on_next_image);
    lv_obj_align_to(next_button, frame, LV_ALIGN_OUT_RIGHT_MID, 24, 0);

    g_album.tip_label = lv_label_create(g_album.screen);
    lv_obj_set_style_text_color(g_album.tip_label, lv_color_hex(0x7F7468), 0);
    lv_obj_set_style_text_font(g_album.tip_label, ui_font_get_22(), 0);
    lv_obj_align_to(g_album.tip_label, frame, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);

    g_album.current_index = 0;
    refresh_album_image();

    return g_album.screen;
}
