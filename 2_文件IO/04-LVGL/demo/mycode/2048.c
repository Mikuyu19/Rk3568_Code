#include "2048.h"
#include "ui_font.h" // 引入自定义字体模块
#include "user_data.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define GRID_SIZE 4
#define TILE_SIZE 60   // 每个方块的尺寸（宽和高）
#define TILE_GAP 10    // 方块之间的间距
#define BOARD_SIZE 300 // 整个棋盘底板的尺寸
// 固定的图片绝对路径前缀（结合你当前的 Ubuntu 虚拟机共享目录）
#define IMAGE_DIR "A:/yu/2048_project/pic"

/* 2048 游戏全局状态结构体 */
typedef struct
{
    lv_obj_t *screen;                           // 2048 整个页面的根屏幕
    lv_obj_t *board_win;                        // 棋盘的底板容器
    lv_obj_t *tile_image[GRID_SIZE][GRID_SIZE]; // 存放 16 个图片控件的指针
    lv_obj_t *score_label;                      // 显示当前分数的标签
    lv_obj_t *best_label;                       // 显示最高分数的标签
    lv_obj_t *status_label;                     // 屏幕底部的状态提示语
    lv_point_t start_point;                     // 记录手指按下的坐标
    lv_point_t end_point;                       // 记录手指松开的坐标
    int game_grid[GRID_SIZE][GRID_SIZE];        // 底层 4x4 逻
    int score;                                  // 当前得分
    int best_score;                             // 历史最高得分
    int game_over;                              // 游戏结束标志位 (1:结束, 0:进行中)
    int game_win;                               // 游戏胜利标志位 (1:胜利, 0:未胜利)
    int win_popup_done;                         // 胜利弹窗是否已经弹过
    int fail_popup_done;                        // 失败弹窗是否已经弹过
} game_2048_t;

static game_2048_t g_game;

static void popup_close_cb(lv_event_t *e)
{
    lv_obj_t *mbox = lv_event_get_user_data(e);
    lv_msgbox_close(mbox);
}

/* 创建通用按钮的封装函数 */
static lv_obj_t *create_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 108, 42);                             // 设置按钮大小
    lv_obj_set_style_radius(btn, 14, 0);                       // 圆角半径
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0); // 浅蓝色背景
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);             // 完全不透明
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);      // 绑定点击事件

    lv_obj_t *lab = lv_label_create(btn); // 按钮上的文本标签
    lv_label_set_text(lab, text);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0); // 22号字体
    lv_obj_center(lab);                                   // 文本居中

    return btn;
}

/* 创建分数显示卡片 */
static lv_obj_t *create_score_card(lv_obj_t *parent, lv_obj_t **out_label)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 150, 60);
    lv_obj_set_style_radius(card, 16, 0);
    lv_obj_set_style_bg_color(card, lv_color_hex(0xBBADA0), 0); // 灰褐色背景
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 0, 0);       // 无边框
    lv_obj_set_style_pad_all(card, 0, 0);            // 无内边距
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE); // 禁用滚动

    *out_label = lv_label_create(card); // 内部的文字标签
    lv_obj_set_style_text_align(*out_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(*out_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(*out_label, ui_font_get_22(), 0);
    lv_obj_center(*out_label);

    return card;
}

/* 快捷设置底部状态提示语 */
static void set_status(const char *text)
{
    lv_label_set_text(g_game.status_label, text);
}

static void show_popup(const char *title, const char *text)
{
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_obj_t *title_label;
    lv_obj_t *text_label;
    lv_obj_t *ok_btn;
    lv_obj_set_style_radius(mbox, 18, 0);
    lv_obj_set_style_bg_opa(mbox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(mbox, 0, 0);

    title_label = lv_msgbox_add_title(mbox, title);
    text_label = lv_msgbox_add_text(mbox, text);
    lv_obj_set_style_text_font(title_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(text_label, ui_font_get_22(), 0);
    ok_btn = lv_msgbox_add_footer_button(mbox, "确定");
    lv_obj_set_style_text_font(ok_btn, ui_font_get_22(), 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(ok_btn, lv_color_black(), 0);
    lv_obj_add_event_cb(ok_btn, popup_close_cb, LV_EVENT_CLICKED, mbox);
    lv_obj_center(mbox);
}

/* 刷新UI上的分数面板 */
static void refresh_score(void)
{
    // 使用格式化输出更新分数文本，\n 实现换行效果
    lv_label_set_text_fmt(g_game.score_label, "当前分数\n%d", g_game.score);
    lv_label_set_text_fmt(g_game.best_label, "最高分\n%d", g_game.best_score);
}

/* 根据矩阵里的数字拼接出图片绝对路径 (如: A:.../pic/2.bmp) */
static void build_img_path(char *buf, size_t size, int value)
{
    size_t dir_len = strlen(IMAGE_DIR);
    if (dir_len > size - 16)
    {
        dir_len = size - 16; // 防止路径超长溢出
    }

    memcpy(buf, IMAGE_DIR, dir_len); // 拷贝目录前缀
    buf[dir_len] = '\0';
    // 拼上文件名
    snprintf(buf + dir_len, size - dir_len, "/%d.bmp", value);
}

/* 将底层游戏矩阵的数据同步渲染到界面的 16 个图片控件上 */
static void update_2048_game(void)
{
    char path[PATH_MAX];

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            build_img_path(path, sizeof(path), g_game.game_grid[i][j]);
            lv_image_set_src(g_game.tile_image[i][j], path); // 刷新单张图片
        }
    }

    refresh_score(); // 同步刷新分数
}

/* 统计当前棋盘上数字为 0（空位）的数量 */
static int get_arr_zero_count(void)
{
    int count = 0;
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (g_game.game_grid[i][j] == 0)
            {
                count++;
            }
        }
    }
    return count;
}

/* 在随机的空位上生成一个 2 或者 4 */
static void rand_num(void)
{
    int count = get_arr_zero_count();
    if (count == 0)
    {
        return; // 没空位了直接返回
    }

    // 随机选择第几个空位产生数字
    int k = rand() % count + 1;
    int q = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (g_game.game_grid[i][j] == 0)
            {
                q++;
                if (q == k) // 找到了被选中的空位
                {
                    // 70% 的概率出 2，30% 的概率出 4 (利用取模 > 2)
                    g_game.game_grid[i][j] = (rand() % 10 > 2) ? 2 : 4;
                    return;
                }
            }
        }
    }
}

/* 初始化游戏数据（只复位逻辑数据，不碰UI界面） */
static void init_2048_game(void)
{
    static int rand_inited = 0;

    // 确保随机数种子只初始化一次
    if (!rand_inited)
    {
        srand((unsigned int)time(NULL));
        rand_inited = 1;
    }

    // 数据清零
    memset(g_game.game_grid, 0, sizeof(g_game.game_grid));
    g_game.score = 0;
    g_game.game_over = 0;
    g_game.game_win = 0;
    g_game.win_popup_done = 0;
    g_game.fail_popup_done = 0;

    // 开局生成两个数字
    rand_num();
    rand_num();
}

/* 更新最高分逻辑 */
static void update_best_score(void)
{
    if (g_game.score > g_game.best_score)
    {
        g_game.best_score = g_game.score;
        user_update_current_best(g_game.best_score);
    }
}

/* 【核心算法】将一维数组中的非 0 数字全部挤到左边，右边补 0 */
static void rm_zero(int temp_arr[], int *flag)
{
    int k = 0; // 记录目前该填入数据的真实下标

    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (temp_arr[i] != 0)
        {
            temp_arr[k] = temp_arr[i]; // 把非0数挪到 k 的位置
            if (k != i)
            {
                temp_arr[i] = 0; // 原来的位置清 0
                *flag = 1;       // 标记数组发生了改变
            }
            k++;
        }
    }
}

/* 【核心算法】将一维数组中相邻且相同的数字合并（从左向右判断） */
static void hebing(int temp_arr[], int *flag)
{
    for (int i = 0; i < GRID_SIZE - 1; i++)
    {
        if (temp_arr[i] == temp_arr[i + 1] && temp_arr[i] != 0)
        {
            temp_arr[i] *= 2;            // 左边数字翻倍
            temp_arr[i + 1] = 0;         // 右边数字清0
            g_game.score += temp_arr[i]; // 累加得分
            update_best_score();
            *flag = 1; // 标记数组发生了改变

            if (temp_arr[i] == 2048)
            {
                g_game.game_win = 1; // 达到 2048，触发胜利标志位
            }
        }
    }
}

/* 向左滑动控制 */
static int slide_left(void)
{
    int temp_arr[GRID_SIZE];
    int flag = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        // 1. 取出一行的数据
        for (int j = 0; j < GRID_SIZE; j++)
        {
            temp_arr[j] = g_game.game_grid[i][j];
        }

        // 2. 核心操作：去0 -> 合并 -> 再去0
        rm_zero(temp_arr, &flag);
        hebing(temp_arr, &flag);
        rm_zero(temp_arr, &flag);

        // 3. 把处理好的数据放回矩阵
        for (int j = 0; j < GRID_SIZE; j++)
        {
            g_game.game_grid[i][j] = temp_arr[j];
        }
    }

    if (flag)
        rand_num(); // 如果盘面发生了有效改变，则生成新数字
    return flag;
}

/* 向右滑动控制（与向左同理，只不过是反着取数据） */
static int slide_right(void)
{
    int temp_arr[GRID_SIZE];
    int flag = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            temp_arr[GRID_SIZE - 1 - j] = g_game.game_grid[i][j]; // 反向装填
        }

        rm_zero(temp_arr, &flag);
        hebing(temp_arr, &flag);
        rm_zero(temp_arr, &flag);

        for (int j = 0; j < GRID_SIZE; j++)
        {
            g_game.game_grid[i][j] = temp_arr[GRID_SIZE - 1 - j]; // 反向放回
        }
    }

    if (flag)
        rand_num();
    return flag;
}

/* 向上滑动控制（按列取数据） */
static int slide_up(void)
{
    int temp_arr[GRID_SIZE];
    int flag = 0;

    for (int j = 0; j < GRID_SIZE; j++)
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            temp_arr[i] = g_game.game_grid[i][j]; // 取出一列
        }

        rm_zero(temp_arr, &flag);
        hebing(temp_arr, &flag);
        rm_zero(temp_arr, &flag);

        for (int i = 0; i < GRID_SIZE; i++)
        {
            g_game.game_grid[i][j] = temp_arr[i]; // 放回一列
        }
    }

    if (flag)
        rand_num();
    return flag;
}

/* 向下滑动控制（反向按列取数据） */
static int slide_down(void)
{
    int temp_arr[GRID_SIZE];
    int flag = 0;

    for (int j = 0; j < GRID_SIZE; j++)
    {
        for (int i = 0; i < GRID_SIZE; i++)
        {
            temp_arr[GRID_SIZE - 1 - i] = g_game.game_grid[i][j];
        }

        rm_zero(temp_arr, &flag);
        hebing(temp_arr, &flag);
        rm_zero(temp_arr, &flag);

        for (int i = 0; i < GRID_SIZE; i++)
        {
            g_game.game_grid[i][j] = temp_arr[GRID_SIZE - 1 - i];
        }
    }

    if (flag)
        rand_num();
    return flag;
}

/* 根据按下和松开的坐标差，计算滑动方向
 * 返回值: 0:无效点击, 1:上, 2:下, 3:左, 4:右
 */
static int get_slide(lv_point_t start_point, lv_point_t end_point)
{
    int x = end_point.x - start_point.x;
    int y = end_point.y - start_point.y;

    // 过滤掉手指点击时的微小抖动（误差10像素以内算作原地点击）
    if (abs(x) < 10 && abs(y) < 10)
    {
        return 0;
    }
    // X轴偏移大于Y轴偏移，判定为横向滑动
    else if (abs(x) > abs(y))
    {
        return x > 0 ? 4 : 3; // 右 : 左
    }
    // 否则为纵向滑动
    else
    {
        return y > 0 ? 2 : 1; // 下 : 上
    }
}

/* 检测游戏是否失败 (即盘面填满且无法再合并) */
static int game_fail(void)
{
    if (get_arr_zero_count() != 0)
    {
        return 0; // 还有空位，未失败
    }

    // 遍历棋盘，看有没有相邻相同的数字
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            // 检查右侧相邻
            if (j < GRID_SIZE - 1 &&
                g_game.game_grid[i][j] == g_game.game_grid[i][j + 1] &&
                g_game.game_grid[i][j] != 0)
            {
                return 0; // 可以合并，未失败
            }

            // 检查下方相邻
            if (i < GRID_SIZE - 1 &&
                g_game.game_grid[i][j] == g_game.game_grid[i + 1][j] &&
                g_game.game_grid[i][j] != 0)
            {
                return 0; // 可以合并，未失败
            }
        }
    }

    return 1; // 死局了，游戏失败
}

/* 根据游戏标志位更新底部提示信息 */
static void check_game_state(void)
{
    if (g_game.game_win)
    {
        set_status("恭喜你，已经合成 2048");
        if (!g_game.win_popup_done)
        {
            g_game.win_popup_done = 1;
            show_popup("游戏胜利", "恭喜你，已经合成 2048");
        }
        return;
    }

    if (game_fail())
    {
        g_game.game_over = 1;
        set_status("游戏结束");
        if (!g_game.fail_popup_done)
        {
            g_game.fail_popup_done = 1;
            show_popup("游戏结束", "当前棋盘已经无法继续移动");
        }
        return;
    }

    set_status("继续游戏");
}

/* 绑定在棋盘底板上的触摸事件回调 */
static void on_board_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    int changed = 0;
    int dir = 0;

    if (code == LV_EVENT_PRESSED) // 手指按下时触发
    {
        lv_indev_get_point(lv_indev_active(), &g_game.start_point); // 记录起点
    }
    else if (code == LV_EVENT_RELEASED) // 手指松开时触发
    {
        if (g_game.game_over)
        {
            set_status("游戏已结束，请点击重置");
            return; // 游戏结束后屏蔽一切滑动操作
        }

        lv_indev_get_point(lv_indev_active(), &g_game.end_point); // 记录终点
        dir = get_slide(g_game.start_point, g_game.end_point);    // 计算滑动方向

        // 根据方向执行对应的滑动算法
        if (dir == 1)
            changed = slide_up();
        else if (dir == 2)
            changed = slide_down();
        else if (dir == 3)
            changed = slide_left();
        else if (dir == 4)
            changed = slide_right();

        if (changed) // 如果底层数据发生了位移或合并
        {
            update_2048_game(); // 刷新屏幕上的图片
            check_game_state(); // 检测死局或胜利
        }
        else if (dir != 0) // 滑了，但是已经滑不动了（靠墙了）
        {
            set_status("这次滑动没有变化");
        }
    }
}

/* 点击重置按钮的回调 */
static void on_reset_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    init_2048_game();   // 初始化底层数据
    update_2048_game(); // 刷新UI
    set_status("滑动棋盘开始游戏");
}

/* 设置基础屏幕风格的共用函数 */
static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFAF8EF), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

/* 2048 应用的创建主入口 */
lv_obj_t *app_2048_create_screen(lv_event_cb_t back_cb)
{
    memset(&g_game, 0, sizeof(g_game)); // 清空全局结构体，防脏数据
    g_game.best_score = user_get_current_best();
    init_2048_game(); // 初始化游戏逻辑盘

    g_game.screen = lv_obj_create(NULL);
    style_base_screen(g_game.screen);

    // 1. 顶部标题
    lv_obj_t *title = lv_label_create(g_game.screen);
    lv_label_set_text(title, "2048");
    lv_obj_set_style_text_color(title, lv_color_hex(0x776E65), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // 2. 左上重置和右上返回按键
    lv_obj_t *back_btn = create_button(g_game.screen, "返回", back_cb);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 16);

    lv_obj_t *reset_btn = create_button(g_game.screen, "重置", on_reset_2048);
    lv_obj_align(reset_btn, LV_ALIGN_TOP_LEFT, 20, 16);

    // 3. 分数显示栏区域
    lv_obj_t *info_row = lv_obj_create(g_game.screen);
    lv_obj_set_size(info_row, 360, 78);
    lv_obj_align(info_row, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_radius(info_row, 18, 0);
    lv_obj_set_style_bg_color(info_row, lv_color_hex(0xEEE4DA), 0);
    lv_obj_set_style_bg_opa(info_row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(info_row, 0, 0);
    lv_obj_set_style_pad_all(info_row, 10, 0);
    lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);

    // 左右两个分数小卡片
    lv_obj_t *score_card = create_score_card(info_row, &g_game.score_label);
    lv_obj_align(score_card, LV_ALIGN_LEFT_MID, 18, 0);

    lv_obj_t *best_card = create_score_card(info_row, &g_game.best_label);
    lv_obj_align(best_card, LV_ALIGN_RIGHT_MID, -18, 0);

    // 4. 棋盘大底板
    g_game.board_win = lv_obj_create(g_game.screen);
    lv_obj_set_size(g_game.board_win, BOARD_SIZE, BOARD_SIZE);
    lv_obj_align_to(g_game.board_win, info_row, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    lv_obj_set_style_radius(g_game.board_win, 18, 0);
    lv_obj_set_style_bg_color(g_game.board_win, lv_color_hex(0xBBADA0), 0);
    lv_obj_set_style_bg_opa(g_game.board_win, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_game.board_win, 0, 0);
    lv_obj_set_style_pad_all(g_game.board_win, 0, 0);
    lv_obj_clear_flag(g_game.board_win, LV_OBJ_FLAG_SCROLLABLE);
    // 【关键绑定】将触摸手势监听事件绑定到整个棋盘面板上
    lv_obj_add_event_cb(g_game.board_win, on_board_event, LV_EVENT_ALL, NULL);

    // 5. 生成 16 个图片控件坑位
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            lv_obj_t *img = lv_image_create(g_game.board_win);
            // 计算每个图片的摆放位置
            lv_obj_set_pos(img, 10 + (TILE_SIZE + TILE_GAP) * j, 10 + (TILE_SIZE + TILE_GAP) * i);
            // 【事件透传机制】让图片不拦截触摸事件，把触摸事件冒泡（透传）给底下的 board_win
            lv_obj_add_flag(img, LV_OBJ_FLAG_EVENT_BUBBLE);
            g_game.tile_image[i][j] = img;
        }
    }

    // 6. 底部状态栏
    g_game.status_label = lv_label_create(g_game.screen);
    lv_obj_set_width(g_game.status_label, 520);
    lv_obj_set_style_text_align(g_game.status_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(g_game.status_label, lv_color_hex(0x8F7A66), 0);
    lv_obj_set_style_text_font(g_game.status_label, ui_font_get_22(), 0);
    lv_obj_align_to(g_game.status_label, g_game.board_win, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);

    update_2048_game(); // 强制刷新一次，将刚生成的数字显示出来
    set_status("滑动棋盘开始游戏");

    return g_game.screen;
}
