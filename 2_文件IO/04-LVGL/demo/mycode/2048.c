#include "2048.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define GRID_SIZE 4
#define TILE_CELL_SIZE 72
#define TILE_GAP 8
#define BOARD_PADDING 12
#define TILE_STEP (TILE_CELL_SIZE + TILE_GAP)
#define BOARD_SIZE (BOARD_PADDING * 2 + GRID_SIZE * TILE_CELL_SIZE + (GRID_SIZE - 1) * TILE_GAP)

/* 2048 棋盘数据。
 * 老师这道练习要求“根据数组值显示对应图片”，
 * 所以这里每个数字都会对应 pic 目录里的一个 bmp 文件：
 * 0 -> 0.bmp
 * 2 -> 2.bmp
 * 4 -> 4.bmp
 * ...
 * 2048 -> 2048.bmp
 *
 * 你后面只要修改这个数组，再调用 refresh_2048_board()，
 * 界面上的图片就会跟着变化。
 */
static int game_grid[GRID_SIZE][GRID_SIZE] = {
    {0, 0, 2, 4},
    {0, 0, 0, 2},
    {2, 2, 0, 0},
    {0, 0, 0, 0}};

/* 2048 模块的运行时状态。
 * 这里集中保存：
 * 1. 两个页面对象：主菜单页、游戏页
 * 2. 动态刷新的标签：当前分数、最高分、状态提示
 * 3. 16 个图片控件的指针
 * 4. 每个格子图片对应的路径缓存
 * 5. 图片目录路径
 * 6. 分数数据
 */
typedef struct
{
    lv_obj_t *menu_screen;
    lv_obj_t *game_screen;
    lv_obj_t *score_label;
    lv_obj_t *best_label;
    lv_obj_t *status_label;
    lv_obj_t *tile_image[GRID_SIZE][GRID_SIZE];
    char tile_path[GRID_SIZE][GRID_SIZE][PATH_MAX];
    char pic_dir[PATH_MAX];
    int score;
    int best_score;
} app_ui_t;

static app_ui_t g_app;

/* 创建一个统一风格的按钮。
 * 这样 Back、Reset 这类按钮都可以复用这一套样式和写法。
 */
static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *button = lv_button_create(parent);
    lv_obj_set_size(button, 110, 42);
    lv_obj_set_style_radius(button, 14, 0);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x4C7CF0), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);

    return button;
}

/* 初始化图片目录。
 *
 * LVGL 从文件系统读取图片时，路径一般要带盘符，例如：
 * A:/mnt/hgfs/.../ubuntu_demo/pic/2.bmp
 *
 * 这里的逻辑是：
 * 1. 如果你手动设置了环境变量 LV_2048_PIC_DIR，就直接用它
 * 2. 否则默认取“当前工作目录 + /pic”
 * 3. 如果当前目录都取不到，就给一个兜底路径
 */
static void init_pic_dir(void)
{
    if (g_app.pic_dir[0] != '\0')
    {
        return;
    }

    const char *custom_dir = getenv("LV_2048_PIC_DIR");
    if (custom_dir && custom_dir[0] != '\0')
    {
        snprintf(g_app.pic_dir, sizeof(g_app.pic_dir), "%s", custom_dir);
        return;
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        snprintf(g_app.pic_dir, sizeof(g_app.pic_dir), "A:%s/pic", cwd);
        return;
    }

    snprintf(g_app.pic_dir, sizeof(g_app.pic_dir), "A:/tmp/pic");
}

/* 根据棋盘坐标拼出当前格子对应的 bmp 文件路径。
 * 例如：
 * game_grid[0][0] = 2048
 * 拼出来的路径就是：
 * A:/.../pic/2048.bmp
 */
static void build_tile_path(int row, int col)
{
    char *dst = g_app.tile_path[row][col];
    size_t dst_size = sizeof(g_app.tile_path[row][col]);
    size_t dir_len = strlen(g_app.pic_dir);

    if (dir_len > dst_size - 16)
    {
        dir_len = dst_size - 16;
    }

    memcpy(dst, g_app.pic_dir, dir_len);
    dst[dir_len] = '\0';

    snprintf(dst + dir_len, dst_size - dir_len, "/%d.bmp", game_grid[row][col]);
}

/* 刷新分数显示区 */
static void refresh_score_labels(void)
{
    lv_label_set_text_fmt(g_app.score_label, "Score\n%d", g_app.score);
    lv_label_set_text_fmt(g_app.best_label, "Best\n%d", g_app.best_score);
}

/* 这是图片版 2048 的核心刷新函数。
 * 普通 2048 常见做法是：
 * - 每个格子放一个 label
 * - label 里显示数字
 *
 * 但老师这题要求的是：
 * - 每个格子显示一张 bmp 图片
 * - 图片由数组值决定
 *
 * 所以这个函数做的就是：
 * 1. 遍历 4x4 数组
 * 2. 给每个格子拼路径
 * 3. 调用 lv_image_set_src() 切换图片
 * 4. 最后刷新分数
 */
static void refresh_2048_board(void)
{
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            build_tile_path(row, col);
            lv_image_set_src(g_app.tile_image[row][col], g_app.tile_path[row][col]);
        }
    }

    refresh_score_labels();
}

/* 初始化当前演示局面。
 * 现在这版不去重置 game_grid，
 * 这样你直接改数组初始值就能立刻看到变化。
 * 这里主要做的是：
 * 1. 分数清零
 * 2. 按当前数组刷新图片
 * 3. 更新底部提示语
 */
static void start_2048_preview(void)
{
    g_app.score = 0;
    refresh_2048_board();
    lv_label_set_text(g_app.status_label, "Image mode ready. Edit game_grid[][] and refresh.");
}

/* 切换回主菜单 */
static void show_menu_screen(void)
{
    lv_screen_load(g_app.menu_screen);
}

/* 进入 2048 游戏页面。
 * 先刷新一次棋盘，确保数组变化已经同步到界面上，
 * 然后再切页。
 */
static void show_2048_screen(void)
{
    refresh_2048_board();
    lv_screen_load(g_app.game_screen);
}

/* 主菜单里点击“2048”按钮时触发 */
static void on_open_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    show_2048_screen();
}

/* 点击 Back 按钮时触发，返回主菜单 */
static void on_back_to_menu(lv_event_t *e)
{
    LV_UNUSED(e);
    show_menu_screen();
}

/* 点击 Reset 按钮时触发，重新显示当前数组状态 */
static void on_restart_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    start_2048_preview();
}

/* 给页面设置统一基础风格：
 * 1. 设置背景色
 * 2. 设置背景完全不透明
 * 3. 关闭页面本身滚动
 */
static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

/* 创建一个“分数卡片”小组件。
 * 返回值是整个卡片对象；
 * out_label 返回的是卡片内部的 label 指针，
 * 方便后面直接更新分数文字。
 */
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
    lv_obj_center(*out_label);

    return card;
}

/* 创建主菜单页面。
 * 当前版本比较简单，主菜单里只有一个“2048”入口。
 * 后面如果你想扩展推箱子、五子棋，也可以继续在这里加按钮。
 */
static void create_menu_screen(void)
{
    g_app.menu_screen = lv_obj_create(NULL);
    style_base_screen(g_app.menu_screen);

    /* 主标题 */
    lv_obj_t *title = lv_label_create(g_app.menu_screen);
    lv_label_set_text(title, "LVGL Game Box");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    /* 列表菜单容器 */
    lv_obj_t *menu_list = lv_list_create(g_app.menu_screen);
    lv_obj_set_size(menu_list, 360, 250);
    lv_obj_align(menu_list, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(menu_list, 20, 0);
    lv_obj_set_style_border_width(menu_list, 0, 0);
    lv_obj_set_style_pad_all(menu_list, 12, 0);

    /* 2048 按钮 */
    lv_obj_t *game_2048_btn = lv_list_add_button(menu_list, NULL, "2048");
    lv_obj_add_event_cb(game_2048_btn, on_open_2048, LV_EVENT_CLICKED, NULL);
}

/* 创建 2048 游戏页面。
 * 页面从上到下分成三层：
 * 1. 顶部栏：标题、返回、重置
 * 2. 信息栏：当前分数、最高分
 * 3. 棋盘区：4x4 格子，每格中间放一张数字图片
 */
static void create_game_screen(void)
{
    g_app.game_screen = lv_obj_create(NULL);
    style_base_screen(g_app.game_screen);

    /* 顶部标题栏 */
    lv_obj_t *header = lv_obj_create(g_app.game_screen);
    lv_obj_set_size(header, lv_pct(94), 66);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_radius(header, 18, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    /* 游戏标题 */
    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 18, 0);

    /* 返回按钮 */
    lv_obj_t *back_button = create_text_button(header, "Back", on_back_to_menu);
    lv_obj_align(back_button, LV_ALIGN_RIGHT_MID, -132, 0);

    /* 重置按钮 */
    lv_obj_t *restart_button = create_text_button(header, "Reset", on_restart_2048);
    lv_obj_align(restart_button, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 分数信息栏 */
    lv_obj_t *info_row = lv_obj_create(g_app.game_screen);
    lv_obj_set_size(info_row, lv_pct(94), 78);
    lv_obj_align_to(info_row, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_radius(info_row, 18, 0);
    lv_obj_set_style_bg_color(info_row, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_bg_opa(info_row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(info_row, 0, 0);
    lv_obj_set_style_pad_all(info_row, 10, 0);
    lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);

    /* 当前分数卡片 */
    lv_obj_t *score_card = create_score_card(info_row, &g_app.score_label);
    lv_obj_align(score_card, LV_ALIGN_LEFT_MID, 18, 0);

    /* 最高分卡片 */
    lv_obj_t *best_card = create_score_card(info_row, &g_app.best_label);
    lv_obj_align(best_card, LV_ALIGN_LEFT_MID, 184, 0);

    /* 棋盘底板 */
    lv_obj_t *board = lv_obj_create(g_app.game_screen);
    lv_obj_set_size(board, BOARD_SIZE, BOARD_SIZE);
    lv_obj_align_to(board, info_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_radius(board, 18, 0);
    lv_obj_set_style_bg_color(board, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(board, 0, 0);
    lv_obj_set_style_pad_all(board, 0, 0);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);

    /* 创建 4x4 棋盘格子 */
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            /* 每个 cell 是一个浅色底格，
             * 作用是给 60x60 bmp 图片提供一个居中的显示槽位。
             */
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

            /* 真正显示数字图片的控件 */
            lv_obj_t *image = lv_image_create(cell);
            lv_obj_set_align(image, LV_ALIGN_CENTER);

            /* 保存图片控件指针，刷新棋盘时要用 */
            g_app.tile_image[row][col] = image;
        }
    }

    /* 棋盘下方的状态提示 */
    g_app.status_label = lv_label_create(g_app.game_screen);
    lv_obj_set_width(g_app.status_label, 560);
    lv_obj_set_style_text_align(g_app.status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_app.status_label, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_text_font(g_app.status_label, &lv_font_montserrat_30, 0);
    lv_obj_align_to(g_app.status_label, board, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
}

/* 2048 模块入口函数。
 * 执行顺序如下：
 * 1. 清空全局状态结构体
 * 2. 初始化随机数种子
 * 3. 初始化最高分和图片目录
 * 4. 创建主菜单和游戏页
 * 5. 按当前数组刷新一遍棋盘
 * 6. 默认显示主菜单
 */
void app_2048_create(void)
{
    memset(&g_app, 0, sizeof(g_app));
    srand((unsigned int)time(NULL));
    g_app.best_score = 0;
    init_pic_dir();

    create_menu_screen();
    create_game_screen();
    start_2048_preview();
    show_menu_screen();
}
