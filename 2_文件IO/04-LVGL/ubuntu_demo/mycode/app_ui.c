#include "app_ui.h"

#include "2048.h"
#include "album.h"
#include "ui_font.h"

#include <string.h>

typedef struct {
    lv_obj_t *menu_screen;
    lv_obj_t *game_2048_screen;
    lv_obj_t *album_screen;
} app_ui_state_t;

static app_ui_state_t g_app_ui;

static lv_obj_t *create_menu_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 260, 68);
    lv_obj_set_style_radius(button, 18, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4C7CF0), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, ui_font_get_26(), 0);
    lv_obj_center(label);

    return button;
}

static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF6F2EA), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

static void on_back_to_menu(lv_event_t *e)
{
    LV_UNUSED(e);
    lv_screen_load(g_app_ui.menu_screen);
}

static void on_open_2048(lv_event_t *e)
{
    LV_UNUSED(e);

    if (g_app_ui.game_2048_screen == NULL) {
        g_app_ui.game_2048_screen = app_2048_create_screen(on_back_to_menu);
    }

    lv_screen_load(g_app_ui.game_2048_screen);
}

static void on_open_album(lv_event_t *e)
{
    LV_UNUSED(e);

    if (g_app_ui.album_screen == NULL) {
        g_app_ui.album_screen = album_create_screen(on_back_to_menu);
    }

    lv_screen_load(g_app_ui.album_screen);
}

static void create_menu_screen(void)
{
    g_app_ui.menu_screen = lv_obj_create(NULL);
    style_base_screen(g_app_ui.menu_screen);

    lv_obj_t *title = lv_label_create(g_app_ui.menu_screen);
    lv_label_set_text(title, "LVGL 作业");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 36);

    lv_obj_t *sub_title = lv_label_create(g_app_ui.menu_screen);
    lv_label_set_text(sub_title, "请选择一个界面");
    lv_obj_set_style_text_color(sub_title, lv_color_hex(0x8A7F73), 0);
    lv_obj_set_style_text_font(sub_title, ui_font_get_22(), 0);
    lv_obj_align_to(sub_title, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t *panel = lv_obj_create(g_app_ui.menu_screen);
    lv_obj_set_size(panel, 360, 240);
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(panel, 24, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xE6DED2), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 18, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *game_btn = create_menu_button(panel, "2048", on_open_2048);
    lv_obj_align(game_btn, LV_ALIGN_TOP_MID, 0, 20);

    lv_obj_t *album_btn = create_menu_button(panel, "电子相册", on_open_album);
    lv_obj_align_to(album_btn, game_btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 24);
}

void app_ui_create(void)
{
    memset(&g_app_ui, 0, sizeof(g_app_ui));
    create_menu_screen();
    lv_screen_load(g_app_ui.menu_screen);
}
