#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "lvgl/examples/lv_examples.h"
#include <stdio.h>

#define GRID_SIZE 4
#define TILE_SIZE 76
#define TILE_GAP 8
#define BOARD_PADDING 12

/* 这个结构体保存整个 2048 小游戏界面的关键状态：
 * 1. 两个页面：主菜单页、游戏页
 * 2. 动态更新的控件：分数、提示文字、16 个格子
 * 3. 2048 的数据：4x4 棋盘、当前分数、最高分
 */
typedef struct
{
    lv_obj_t *menu_screen;
    lv_obj_t *game_screen;
    lv_obj_t *score_label;
    lv_obj_t *best_label;
    lv_obj_t *status_label;
    lv_obj_t *tile_panels[GRID_SIZE][GRID_SIZE];
    lv_obj_t *tile_labels[GRID_SIZE][GRID_SIZE];
    int board[GRID_SIZE][GRID_SIZE];
    int score;
    int best_score;
} app_ui_t;

static app_ui_t g_app;

static const char *getenv_default(const char *name, const char *dflt)
{
    /* 优先读取环境变量，没有时使用默认值 */
    return getenv(name) ?: dflt;
}

#if LV_USE_LINUX_FBDEV
static void lv_linux_disp_init(void)
{
    /* framebuffer 模式一般连接 /dev/fb0 */
    const char *device = getenv_default("LV_LINUX_FBDEV_DEVICE", "/dev/fb0");
    lv_display_t *disp = lv_linux_fbdev_create();

    /* 把 framebuffer 设备交给 LVGL 作为显示输出 */
    lv_linux_fbdev_set_file(disp, device);

    /* 注册输入设备，这里默认写死为触摸节点 /dev/input/event6 */
    lv_indev_t *indev = lv_evdev_create(LV_INDEV_TYPE_POINTER, "/dev/input/event6");
}
#elif LV_USE_LINUX_DRM
static void lv_linux_disp_init(void)
{
    /* 如果后面切换到 DRM 显示，会执行这一分支 */
    const char *device = getenv_default("LV_LINUX_DRM_CARD", "/dev/dri/card0");
    lv_display_t *disp = lv_linux_drm_create();

    lv_linux_drm_set_file(disp, device, -1);
}
#elif LV_USE_SDL
static void lv_linux_disp_init(void)
{
    /* SDL 模式主要用于 Ubuntu/PC 开窗口调试 */
    const int width = atoi(getenv("LV_SDL_VIDEO_WIDTH") ?: "1024");
    const int height = atoi(getenv("LV_SDL_VIDEO_HEIGHT") ?: "600");

    lv_sdl_window_create(width, height);
}
#else
#error Unsupported configuration
#endif

static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    /* 封装一个通用文字按钮，避免 Back/Restart 重复写样式 */
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

static lv_color_t get_tile_bg_color(int value)
{
    /* 2048 每个数字对应不同背景色 */
    switch (value)
    {
    case 0:
        return lv_color_hex(0xCDC1B4);
    case 2:
        return lv_color_hex(0xEEE4DA);
    case 4:
        return lv_color_hex(0xEDE0C8);
    case 8:
        return lv_color_hex(0xF2B179);
    case 16:
        return lv_color_hex(0xF59563);
    case 32:
        return lv_color_hex(0xF67C5F);
    case 64:
        return lv_color_hex(0xF65E3B);
    case 128:
        return lv_color_hex(0xEDCF72);
    case 256:
        return lv_color_hex(0xEDCC61);
    case 512:
        return lv_color_hex(0xEDC850);
    case 1024:
        return lv_color_hex(0xEDC53F);
    default:
        return lv_color_hex(0xEDC22E);
    }
}

static lv_color_t get_tile_text_color(int value)
{
    /* 小数字用深色文字，大数字用白色文字 */
    return value <= 4 ? lv_color_hex(0x776E65) : lv_color_white();
}

static void refresh_score_labels(void)
{
    /* 当前分数和最高分变化后，统一刷新到界面上 */
    lv_label_set_text_fmt(g_app.score_label, "Score\n%d", g_app.score);
    lv_label_set_text_fmt(g_app.best_label, "Best\n%d", g_app.best_score);
}

static void refresh_2048_board(void)
{
    /* 把 board[4][4] 数组里的数据同步到 16 个格子控件 */
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            const int value = g_app.board[row][col];

            /* 先更新颜色 */
            lv_obj_set_style_bg_color(g_app.tile_panels[row][col], get_tile_bg_color(value), 0);
            lv_obj_set_style_text_color(g_app.tile_labels[row][col], get_tile_text_color(value), 0);

            if (value == 0)
            {
                /* 空格子不显示文字 */
                lv_label_set_text(g_app.tile_labels[row][col], "");
            }
            else
            {
                /* 非空格子显示具体数字 */
                lv_label_set_text_fmt(g_app.tile_labels[row][col], "%d", value);
            }
        }
    }

    refresh_score_labels();
}

static void clear_board(void)
{
    /* 把整个 4x4 棋盘数据清零 */
    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            g_app.board[row][col] = 0;
        }
    }
}

static bool spawn_random_tile(void)
{
    /* 收集所有空格子，然后随机选择一个生成新数字 */
    int empty_row[GRID_SIZE * GRID_SIZE];
    int empty_col[GRID_SIZE * GRID_SIZE];
    int count = 0;

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            if (g_app.board[row][col] == 0)
            {
                empty_row[count] = row;
                empty_col[count] = col;
                count++;
            }
        }
    }

    if (count == 0)
    {
        return false;
    }

    int index = rand() % count;
    /* 常见 2048 规则：大概率生成 2，小概率生成 4 */
    g_app.board[empty_row[index]][empty_col[index]] = (rand() % 10 == 0) ? 4 : 2;
    return true;
}

static void start_2048_preview(void)
{
    /* 当前这一步只是初始化棋盘预览：
     * 1. 清空棋盘
     * 2. 放两个随机数字
     * 3. 刷新界面
     * 真正的上下左右合并逻辑后面再补
     */
    clear_board();
    g_app.score = 0;
    spawn_random_tile();
    spawn_random_tile();
    refresh_2048_board();
    lv_label_set_text(g_app.status_label, "Framework ready. Next: gesture + merge logic.");
}

static void show_menu_screen(void)
{
    /* 切换到主菜单页 */
    lv_screen_load(g_app.menu_screen);
}

static void show_2048_screen(void)
{
    /* 进入 2048 页面前先初始化一局 */
    start_2048_preview();
    lv_screen_load(g_app.game_screen);
}

static void on_open_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    show_2048_screen();
}

static void on_back_to_menu(lv_event_t *e)
{
    LV_UNUSED(e);
    show_menu_screen();
}

static void on_restart_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    start_2048_preview();
}

static void style_base_screen(lv_obj_t *screen)
{
    /* 给页面统一设置背景色，并关闭滚动 */
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *create_score_card(lv_obj_t *parent, lv_obj_t **out_label)
{
    /* 创建一个分数卡片，并把内部 label 传回去，方便后续直接刷新 */
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

static void create_menu_screen(void)
{
    /* 创建主菜单页面：
     * 现在先把 2048 做成可点击入口，
     * 其他游戏先用禁用按钮占位，后面再扩展。
     */
    g_app.menu_screen = lv_obj_create(NULL);
    style_base_screen(g_app.menu_screen);

    /* 标题 */
    lv_obj_t *title = lv_label_create(g_app.menu_screen);
    lv_label_set_text(title, "LVGL Game Box");
    lv_obj_set_style_text_font(title, LV_FONT_DEFAULT, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    /* 副标题 */
    lv_obj_t *subtitle = lv_label_create(g_app.menu_screen);
    lv_label_set_text(subtitle, "Phase 1 focus: build the 2048 framework first.");
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x8F7A66), 0);
    lv_obj_align_to(subtitle, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    /* 菜单列表容器 */
    lv_obj_t *menu_list = lv_list_create(g_app.menu_screen);
    lv_obj_set_size(menu_list, 360, 250);
    lv_obj_align(menu_list, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_radius(menu_list, 20, 0);
    lv_obj_set_style_border_width(menu_list, 0, 0);
    lv_obj_set_style_pad_all(menu_list, 12, 0);

    /* 2048 按钮，点击后进入游戏页 */
    lv_obj_t *game_2048_btn = lv_list_add_button(menu_list, NULL, "2048");
    lv_obj_add_event_cb(game_2048_btn, on_open_2048, LV_EVENT_CLICKED, NULL);

    /* 下面几个按钮先占位，后续再做 */
    lv_obj_t *sokoban_btn = lv_list_add_button(menu_list, NULL, "Sokoban (next)");
    lv_obj_add_state(sokoban_btn, LV_STATE_DISABLED);

    lv_obj_t *gomoku_btn = lv_list_add_button(menu_list, NULL, "Gomoku (next)");
    lv_obj_add_state(gomoku_btn, LV_STATE_DISABLED);

    lv_obj_t *chess_btn = lv_list_add_button(menu_list, NULL, "Chinese Chess (extension)");
    lv_obj_add_state(chess_btn, LV_STATE_DISABLED);

    /* 页面底部提示：说明当前开发到哪一步 */
    lv_obj_t *tip = lv_label_create(g_app.menu_screen);
    lv_obj_set_width(tip, 500);
    lv_label_set_text(
        tip,
        "Current milestone:\n"
        "1. menu navigation\n"
        "2. 2048 board UI\n"
        "3. score / restart hooks\n"
        "4. merge logic in the next step");
    lv_obj_set_style_text_align(tip, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(tip, lv_color_hex(0x8F7A66), 0);
    lv_obj_align(tip, LV_ALIGN_BOTTOM_MID, 0, -24);
}

static void create_game_screen(void)
{
    /* 创建 2048 游戏页面 */
    g_app.game_screen = lv_obj_create(NULL);
    style_base_screen(g_app.game_screen);

    /* 顶部栏：游戏标题 + 返回按钮 + 重开按钮 */
    lv_obj_t *header = lv_obj_create(g_app.game_screen);
    lv_obj_set_size(header, lv_pct(94), 66);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 12);
    lv_obj_set_style_radius(header, 18, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 10, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 18, 0);

    /* 返回主菜单 */
    lv_obj_t *back_button = create_text_button(header, "Back", on_back_to_menu);
    lv_obj_align(back_button, LV_ALIGN_RIGHT_MID, -132, 0);

    /* 重新开始当前这一局 */
    lv_obj_t *restart_button = create_text_button(header, "Restart", on_restart_2048);
    lv_obj_align(restart_button, LV_ALIGN_RIGHT_MID, -10, 0);

    /* 信息栏：分数、最高分、提示语 */
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

    /* 右侧提示语 */
    lv_obj_t *helper = lv_label_create(info_row);
    lv_label_set_text(helper, "This page is ready for\nswipe input and merge rules.");
    lv_obj_set_style_text_color(helper, lv_color_hex(0x8F7A66), 0);
    lv_obj_align(helper, LV_ALIGN_RIGHT_MID, -24, 0);

    /* 棋盘底板 */
    lv_obj_t *board = lv_obj_create(g_app.game_screen);
    lv_obj_set_size(board, 336, 336);
    lv_obj_align_to(board, info_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_radius(board, 18, 0);
    lv_obj_set_style_bg_color(board, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(board, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(board, 0, 0);
    lv_obj_set_style_pad_all(board, 0, 0);
    lv_obj_clear_flag(board, LV_OBJ_FLAG_SCROLLABLE);

    for (int row = 0; row < GRID_SIZE; row++)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            /* 创建一个格子背景块 */
            lv_obj_t *tile = lv_obj_create(board);
            lv_obj_set_size(tile, TILE_SIZE, TILE_SIZE);
            lv_obj_set_pos(
                tile,
                BOARD_PADDING + col * (TILE_SIZE + TILE_GAP),
                BOARD_PADDING + row * (TILE_SIZE + TILE_GAP));
            lv_obj_set_style_radius(tile, 12, 0);
            lv_obj_set_style_border_width(tile, 0, 0);
            lv_obj_set_style_pad_all(tile, 0, 0);
            lv_obj_clear_flag(tile, LV_OBJ_FLAG_SCROLLABLE);

            /* 在格子中间放一个数字标签 */
            lv_obj_t *label = lv_label_create(tile);
            lv_obj_set_style_text_color(label, lv_color_hex(0x776E65), 0);
            lv_obj_center(label);

            /* 保存指针，后面刷新棋盘时就能直接访问 */
            g_app.tile_panels[row][col] = tile;
            g_app.tile_labels[row][col] = label;
        }
    }

    /* 棋盘下方状态提示 */
    g_app.status_label = lv_label_create(g_app.game_screen);
    lv_obj_set_width(g_app.status_label, 560);
    lv_obj_set_style_text_align(g_app.status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_app.status_label, lv_color_hex(0x8F7A66), 0);
    lv_obj_align_to(g_app.status_label, board, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
}

static void app_create(void)
{
    /* 应用总初始化：
     * 1. 初始化随机数种子
     * 2. 初始化最高分
     * 3. 创建各页面
     * 4. 默认显示主菜单
     */
    srand((unsigned int)time(NULL));
    g_app.best_score = 0;

    create_menu_screen();
    create_game_screen();
    show_menu_screen();
}

int main(void)
{
    /* LVGL 程序的标准流程：
     * 1. 初始化 LVGL 核心
     * 2. 初始化显示和输入设备
     * 3. 创建自己的界面
     * 4. 循环调用 lv_timer_handler() 处理刷新和事件
     */
    lv_init();
    lv_linux_disp_init();

    // lv_demo_widgets();
    // lv_demo_widgets_start_slideshow();
    /* 切换到我们自己的 2048 项目入口 */
    // app_create();

    while (1)
    {
        /* LVGL 的任务调度、动画、输入处理都依赖这个函数 */
        lv_timer_handler();
        /* 略微休眠，避免 CPU 空转 */
        usleep(5000);
    }
    return 0;
}
