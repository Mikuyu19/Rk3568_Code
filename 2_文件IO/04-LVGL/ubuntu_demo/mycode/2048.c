#include "2048.h"
#include "ui_font.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GRID_SIZE 4
#define TILE_CELL_SIZE 72
#define TILE_GAP 8
#define BOARD_PADDING 12
#define TILE_STEP (TILE_CELL_SIZE + TILE_GAP)
#define BOARD_SIZE (BOARD_PADDING * 2 + GRID_SIZE * TILE_CELL_SIZE + (GRID_SIZE - 1) * TILE_GAP)

/* 2048 棋盘数据。
 * 这里的数字会映射成 pic 目录里的 bmp 文件：
 * 0 -> 0.bmp
 * 2 -> 2.bmp
 * ...
 * 2048 -> 2048.bmp
 */
static int game_grid[GRID_SIZE][GRID_SIZE] = {
    {0, 0, 2, 4},
    {0, 0, 0, 2},
    {2, 2, 0, 0},
    {0, 0, 0, 0}
};

typedef struct {
    lv_obj_t *screen;
    lv_obj_t *score_label;
    lv_obj_t *best_label;
    lv_obj_t *status_label;
    lv_obj_t *tile_image[GRID_SIZE][GRID_SIZE];
    char tile_path[GRID_SIZE][GRID_SIZE][PATH_MAX];
    char pic_dir[PATH_MAX];
    int score;
    int best_score;
} game_2048_t;

static game_2048_t g_game_2048;

static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 108, 42);
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
    if (g_game_2048.pic_dir[0] != '\0') {
        return;
    }

    const char *custom_dir = getenv("LV_2048_PIC_DIR");
    if (custom_dir && custom_dir[0] != '\0') {
        snprintf(g_game_2048.pic_dir, sizeof(g_game_2048.pic_dir), "%s", custom_dir);
        return;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(g_game_2048.pic_dir, sizeof(g_game_2048.pic_dir), "A:%s/pic", cwd);
        return;
    }

    snprintf(g_game_2048.pic_dir, sizeof(g_game_2048.pic_dir), "A:/tmp/pic");
}

static void build_tile_path(int row, int col)
{
    char *dst = g_game_2048.tile_path[row][col];
    size_t dst_size = sizeof(g_game_2048.tile_path[row][col]);
    size_t dir_len = strlen(g_game_2048.pic_dir);

    if (dir_len > dst_size - 16) {
        dir_len = dst_size - 16;
    }

    memcpy(dst, g_game_2048.pic_dir, dir_len);
    dst[dir_len] = '\0';

    snprintf(dst + dir_len, dst_size - dir_len, "/%d.bmp", game_grid[row][col]);
}

static void refresh_score_labels(void)
{
    lv_label_set_text_fmt(g_game_2048.score_label, "当前分数\n%d", g_game_2048.score);
    lv_label_set_text_fmt(g_game_2048.best_label, "最高分\n%d", g_game_2048.best_score);
}

/* 根据数组值刷新 16 个图片格子。 */
static void refresh_2048_board(void)
{
    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            build_tile_path(row, col);
            lv_image_set_src(g_game_2048.tile_image[row][col], g_game_2048.tile_path[row][col]);
        }
    }

    refresh_score_labels();
}

/* 当前重置只把分数清零并重新显示数组内容。
 * 后面如果你想做真正的 2048 逻辑，可以在这里恢复默认棋盘。
 */
static void reset_2048_preview(void)
{
    g_game_2048.score = 0;
    refresh_2048_board();
    lv_label_set_text(g_game_2048.status_label, "2048 图片演示界面");
}

static void on_reset_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    reset_2048_preview();
}

static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *create_score_card(lv_obj_t *parent, lv_obj_t **out_label)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 150, 60);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    *out_label = lv_label_create(card);
    lv_obj_set_style_text_align(*out_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(*out_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(*out_label, ui_font_get_22(), 0);
    lv_obj_center(*out_label);

    return card;
}

lv_obj_t *app_2048_create_screen(lv_event_cb_t back_cb)
{
    memset(&g_game_2048, 0, sizeof(g_game_2048));
    init_pic_dir();

    g_game_2048.screen = lv_obj_create(NULL);
    style_base_screen(g_game_2048.screen);

    lv_obj_t *title = lv_label_create(g_game_2048.screen);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

    /* 返回按钮放在右上角 */
    lv_obj_t *back_button = create_text_button(g_game_2048.screen, "返回", back_cb);
    lv_obj_align(back_button, LV_ALIGN_TOP_RIGHT, -20, 16);

    /* Reset 按钮单独放左上角，避免和返回按钮挤在一起 */
    lv_obj_t *reset_button = create_text_button(g_game_2048.screen, "重置", on_reset_2048);
    lv_obj_align(reset_button, LV_ALIGN_TOP_LEFT, 20, 16);

    lv_obj_t *info_row = lv_obj_create(g_game_2048.screen);
    lv_obj_set_size(info_row, lv_pct(94), 78);
    lv_obj_align(info_row, LV_ALIGN_TOP_MID, 0, 74);
    lv_obj_set_style_radius(info_row, 18, 0);
    lv_obj_set_style_bg_color(info_row, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_bg_opa(info_row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(info_row, 0, 0);
    lv_obj_set_style_pad_all(info_row, 10, 0);
    lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *score_card = create_score_card(info_row, &g_game_2048.score_label);
    lv_obj_align(score_card, LV_ALIGN_LEFT_MID, 18, 0);

    lv_obj_t *best_card = create_score_card(info_row, &g_game_2048.best_label);
    lv_obj_align(best_card, LV_ALIGN_LEFT_MID, 184, 0);

    lv_obj_t *board = lv_obj_create(g_game_2048.screen);
    lv_obj_set_size(board, BOARD_SIZE, BOARD_SIZE);
    lv_obj_align_to(board, info_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_radius(board, 18, 0);
    lv_obj_set_style_bg_color(board, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(board, 0, 0);
    lv_obj_set_style_pad_all(board, 0, 0);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);

    for (int row = 0; row < GRID_SIZE; row++) {
        for (int col = 0; col < GRID_SIZE; col++) {
            lv_obj_t *cell = lv_obj_create(board);
            lv_obj_set_size(cell, TILE_CELL_SIZE, TILE_CELL_SIZE);
            lv_obj_set_pos(
                cell,
                BOARD_PADDING + col * TILE_STEP,
                BOARD_PADDING + row * TILE_STEP);
            lv_obj_set_style_radius(cell, 12, 0);
            lv_obj_set_style_bg_color(cell, lv_color_hex(0xCDC1B4), 0);
            lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(cell, 0, 0);
            lv_obj_set_style_pad_all(cell, 0, 0);
            lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t *image = lv_image_create(cell);
            lv_obj_set_align(image, LV_ALIGN_CENTER);
            g_game_2048.tile_image[row][col] = image;
        }
    }

    g_game_2048.status_label = lv_label_create(g_game_2048.screen);
    lv_obj_set_width(g_game_2048.status_label, 560);
    lv_obj_set_style_text_align(g_game_2048.status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_game_2048.status_label, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_text_font(g_game_2048.status_label, ui_font_get_22(), 0);
    lv_obj_align_to(g_game_2048.status_label, board, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);

    reset_2048_preview();
    return g_game_2048.screen;
}
