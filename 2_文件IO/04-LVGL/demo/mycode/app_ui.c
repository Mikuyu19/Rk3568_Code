#include "app_ui.h"

#include "2048.h"
#include "album.h"
#include "medical.h"
#include "ui_font.h"
#include "user_data.h"

#include <string.h>

/* App 总体 UI 状态结构体，集中管理界面指针 */
typedef struct
{
    lv_obj_t *login_screen;     // 登录页面
    lv_obj_t *menu_screen;      // 主菜单页面
    lv_obj_t *rank_screen;      // 排行榜页面
    lv_obj_t *game_2048_screen; // 2048 游戏页面
    lv_obj_t *album_screen;     // 电子相册页面
    lv_obj_t *medical_screen;   // 医疗叫号页面

    lv_obj_t *user_ta;   // 账号输入框
    lv_obj_t *pwd_ta;    // 密码输入框
    lv_obj_t *msg_label; // 错误提示文本
    lv_obj_t *kb;        // 虚拟键盘

    lv_obj_t *current_user_label; // 主菜单显示当前登录用户名的标签
    lv_obj_t *rank_panel;         // 排行榜界面中用于滚动的中间大卡片区域
} app_ui_t;

static app_ui_t g_ui;

/* Msgbox 弹窗的关闭按钮回调函数 */
static void popup_close_cb(lv_event_t *e)
{
    // 从事件数据中获取刚才创建的 mbox 对象的指针，然后关闭它
    lv_obj_t *mbox = lv_event_get_user_data(e);
    lv_msgbox_close(mbox);
}

/* 创建统一的大按钮 */
static lv_obj_t *create_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_button_create(parent);
    lv_obj_set_size(btn, 180, 54);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lab = lv_label_create(btn);
    lv_label_set_text(lab, text);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_center(lab);

    return btn;
}

/* 统一基础屏幕风格 */
static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF6F2EA), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
}

/* 显示通用弹出提示框 (Msgbox) */
static void show_popup(const char *title, const char *text)
{
    // 创建一个消息框 (传 NULL 默认在当前活动屏幕的最顶层创建)
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_obj_set_style_radius(mbox, 18, 0);
    lv_obj_set_style_bg_opa(mbox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(mbox, 0, 0);

    // 填充标题和内容
    lv_obj_t *title_label = lv_msgbox_add_title(mbox, title);
    lv_obj_t *text_label = lv_msgbox_add_text(mbox, text);
    lv_obj_set_style_text_font(title_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(text_label, ui_font_get_22(), 0);

    // 添加底部的确定按钮
    lv_obj_t *ok_btn = lv_msgbox_add_footer_button(mbox, "确定");
    lv_obj_set_style_text_font(ok_btn, ui_font_get_22(), 0);
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(ok_btn, lv_color_black(), 0);

    // 绑定关闭事件，把 mbox 指针当做参数传过去，以便在回调里销毁它
    lv_obj_add_event_cb(ok_btn, popup_close_cb, LV_EVENT_CLICKED, mbox);
    lv_obj_center(mbox); // 弹窗居中
}

/* 收起键盘 */
static void hide_keyboard(void)
{
    if (g_ui.kb == NULL)
        return;
    lv_keyboard_set_textarea(g_ui.kb, NULL);
    lv_obj_add_flag(g_ui.kb, LV_OBJ_FLAG_HIDDEN);
}

/* 清空登录框的残余输入 */
static void clear_login_input(void)
{
    lv_textarea_set_text(g_ui.user_ta, "");
    lv_textarea_set_text(g_ui.pwd_ta, "");
    lv_obj_add_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN); // 隐藏报错信息
}

static void delete_user_screens(void)
{
    if (g_ui.game_2048_screen != NULL)
    {
        // lv_obj_delete(g_ui.game_2048_screen); // 彻底销毁页面及上面的所有控件
        lv_obj_delete_async(g_ui.game_2048_screen); // 换成 _async 异步安全删除
        g_ui.game_2048_screen = NULL;
    }

    if (g_ui.album_screen != NULL)
    {
        lv_obj_delete_async(g_ui.album_screen); // 换成 _async 异步安全删除
        g_ui.album_screen = NULL;
    }

    if (g_ui.medical_screen != NULL)
    {
        lv_obj_delete_async(g_ui.medical_screen);
        g_ui.medical_screen = NULL;
    }

    if (g_ui.rank_screen != NULL)
    {
        lv_obj_delete_async(g_ui.rank_screen);
        g_ui.rank_screen = NULL;
        g_ui.rank_panel = NULL;
    }
}

/* 更新主界面的登录欢迎语 */
static void update_current_user_label(void)
{
    // 调用后端接口获取名字并显示
    lv_label_set_text_fmt(g_ui.current_user_label, "当前用户：%s", user_get_current_name());
}

/* 文本框交互事件 */
static void on_ta_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(g_ui.kb, ta);
        lv_obj_clear_flag(g_ui.kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN);
    }
}

/* 键盘按键事件 */
static void on_kb_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        hide_keyboard();
    }
}

/* 返回菜单回调 */
static void on_back_to_menu(lv_event_t *e)
{
    LV_UNUSED(e);
    lv_screen_load(g_ui.menu_screen);
}

/* 进入 2048 */
static void on_open_2048(lv_event_t *e)
{
    LV_UNUSED(e);
    if (g_ui.game_2048_screen == NULL)
    {
        g_ui.game_2048_screen = app_2048_create_screen(on_back_to_menu);
    }
    // lv_screen_load(g_ui.game_2048_screen);
    lv_screen_load_anim(g_ui.game_2048_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/* 进入 相册 */
static void on_open_album(lv_event_t *e)
{
    LV_UNUSED(e);
    if (g_ui.album_screen == NULL)
    {
        g_ui.album_screen = album_create_screen(on_back_to_menu);
    }
    // lv_screen_load(g_ui.album_screen);
    lv_screen_load_anim(g_ui.album_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/* 进入 医疗叫号系统 */
static void on_open_medical(lv_event_t *e)
{
    LV_UNUSED(e);
    if (g_ui.medical_screen == NULL)
    {
        g_ui.medical_screen = medical_create_screen(on_back_to_menu);
    }
    // lv_screen_load(g_ui.medical_screen);
    lv_screen_load_anim(g_ui.medical_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/* 读取后端排行榜数据并在界面上动态生成列表 */
static void refresh_rank_list(void)
{
    rank_item_t list[USER_MAX_COUNT];
    int count = user_get_rank_list(list, USER_MAX_COUNT); // 调用排序后返回数量

    lv_obj_clean(g_ui.rank_panel); // 清空面板上旧的排名标签

    // 根据用户数量，动态生成一行行的排行榜标签
    for (int i = 0; i < count; i++)
    {
        lv_obj_t *lab = lv_label_create(g_ui.rank_panel);
        lv_label_set_text_fmt(lab, "%d. %s  %d", i + 1, list[i].username, list[i].best_score);
        lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(lab, lv_color_hex(0x5E5248), 0);
        // 根据 i 算绝对 Y 坐标，形成一个纵向列表
        lv_obj_align(lab, LV_ALIGN_TOP_LEFT, 16, 16 + i * 34);
    }
}

/* 构建排行榜页面 */
static void create_rank_screen(void)
{
    g_ui.rank_screen = lv_obj_create(NULL);
    style_base_screen(g_ui.rank_screen);

    lv_obj_t *title = lv_label_create(g_ui.rank_screen);
    lv_label_set_text(title, "排行榜");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    lv_obj_t *back_btn = create_button(g_ui.rank_screen, "返回", on_back_to_menu);
    lv_obj_set_size(back_btn, 120, 46);
    lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -20, 20);

    g_ui.rank_panel = lv_obj_create(g_ui.rank_screen);
    lv_obj_set_size(g_ui.rank_panel, 420, 340);
    lv_obj_align(g_ui.rank_panel, LV_ALIGN_CENTER, 0, 28);
    lv_obj_set_style_radius(g_ui.rank_panel, 20, 0);
    lv_obj_set_style_bg_color(g_ui.rank_panel, lv_color_hex(0xEAE1D4), 0);
    lv_obj_set_style_bg_opa(g_ui.rank_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_ui.rank_panel, 0, 0);
    lv_obj_set_style_pad_all(g_ui.rank_panel, 0, 0);
    lv_obj_set_scroll_dir(g_ui.rank_panel, LV_DIR_VER); // 仅允许上下滚动面板

    refresh_rank_list(); // 初始化完毕后立马拉取一次数据填充面板
}

/* 进入 排行榜 */
static void on_open_rank(lv_event_t *e)
{
    LV_UNUSED(e);

    // 懒加载模式：创建过就直接刷新数据，没创建过就现成搭界面
    if (g_ui.rank_screen == NULL)
    {
        create_rank_screen();
    }
    else
    {
        refresh_rank_list();
    }

    // lv_screen_load(g_ui.rank_screen);
    lv_screen_load_anim(g_ui.rank_screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

/* 登出账号操作 */
static void on_logout(lv_event_t *e)
{
    LV_UNUSED(e);

    user_logout();         // 后端状态登出
    delete_user_screens(); // 清空该用户打开的所有私人页面和数据缓存
    clear_login_input();   // 清空登录框上的账号密码痕迹
    hide_keyboard();       // 收起键盘

    lv_screen_load(g_ui.login_screen); // 踢回登录页
    // 1. 【核心修复】必须先将画面切回登录页，让旧页面失去 active (激活) 状态！
    // 登出动作直接使用 lv_screen_load 硬切，避免动画期间旧页面被销毁导致崩溃
    // 2. 等安全切走之后，再执行清空销毁操作
    delete_user_screens();
}

/* 构建主菜单 */
static void create_menu_screen(void)
{
    g_ui.menu_screen = lv_obj_create(NULL);
    style_base_screen(g_ui.menu_screen);

    lv_obj_t *title = lv_label_create(g_ui.menu_screen);
    lv_label_set_text(title, "功能选择");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

    g_ui.current_user_label = lv_label_create(g_ui.menu_screen);
    lv_obj_set_style_text_color(g_ui.current_user_label, lv_color_hex(0x7B6F63), 0);
    lv_obj_set_style_text_font(g_ui.current_user_label, ui_font_get_22(), 0);
    lv_obj_align(g_ui.current_user_label, LV_ALIGN_TOP_LEFT, 24, 26); // 左上角展示登录用户

    lv_obj_t *logout_btn = create_button(g_ui.menu_screen, "退出登录", on_logout);
    lv_obj_set_size(logout_btn, 140, 46);
    lv_obj_align(logout_btn, LV_ALIGN_TOP_RIGHT, -20, 20); // 右上角放置退出按键

    lv_obj_t *game_btn = create_button(g_ui.menu_screen, "2048", on_open_2048);
    lv_obj_align(game_btn, LV_ALIGN_CENTER, 0, -70);

    lv_obj_t *album_btn = create_button(g_ui.menu_screen, "电子相册", on_open_album);
    lv_obj_align(album_btn, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *medical_btn = create_button(g_ui.menu_screen, "医疗叫号系统", on_open_medical);
    lv_obj_align(medical_btn, LV_ALIGN_CENTER, 0, 60);

    lv_obj_t *rank_btn = create_button(g_ui.menu_screen, "排行榜", on_open_rank);
    lv_obj_align(rank_btn, LV_ALIGN_CENTER, 0, 130);
}

/* 尝试登录 */
static void on_login(lv_event_t *e)
{
    LV_UNUSED(e);

    const char *user = lv_textarea_get_text(g_ui.user_ta);
    const char *pwd = lv_textarea_get_text(g_ui.pwd_ta);

    hide_keyboard();

    if (user_login(user, pwd))
    { // 调用后端验证
        lv_obj_add_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN);
#if 0
        // 需要刷新主菜单的用户名标签
        update_current_user_label();

        // 跳转到大厅(主菜单)
        lv_screen_load_anim(g_ui.menu_screen, LV_SCR_LOAD_ANIM_FADE_ON,300,0,false);
#endif
#if 1
        // 创建并直接加载医疗叫号页面
        if (g_ui.medical_screen == NULL)
        {
            // 注意这里：我们将右上角按钮绑定的回调函数改成了 on_logout (退出登录)
            // 这样在医疗界面点右上角就会直接退回登录页
            g_ui.medical_screen = medical_create_screen(on_logout);
        }

        // lv_screen_load(g_ui.medical_screen); // 直接进入医疗系统
        lv_screen_load_anim(g_ui.medical_screen, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, false);
#endif
    }
    else
    {
        lv_label_set_text(g_ui.msg_label, "账号或密码错误");
        lv_obj_clear_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN); // 显示红字报错
    }
}

/* 尝试注册新账号 */
static void on_register(lv_event_t *e)
{
    LV_UNUSED(e);

    const char *user = lv_textarea_get_text(g_ui.user_ta);
    const char *pwd = lv_textarea_get_text(g_ui.pwd_ta);

    hide_keyboard();

    // 请求后端注册写入文件
    if (user_register(user, pwd))
    {
        lv_label_set_text(g_ui.msg_label, "注册成功，请登录");
        lv_obj_clear_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN);
        // 使用弹窗给予强烈的正向反馈
        show_popup("注册成功", "账号已经创建，可以直接登录");
    }
    else
    {
        lv_label_set_text(g_ui.msg_label, "注册失败，用户名可能已存在");
        lv_obj_clear_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN);
        show_popup("注册失败", "用户名已存在或输入不合法");
    }
}

/* 构建登录页 */
static void create_login_screen(void)
{
    g_ui.login_screen = lv_obj_create(NULL);
    style_base_screen(g_ui.login_screen);

    lv_obj_t *title = lv_label_create(g_ui.login_screen);
    lv_label_set_text(title, "登录");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 48);

    lv_obj_t *panel = lv_obj_create(g_ui.login_screen);
    lv_obj_set_size(panel, 360, 270);
    lv_obj_align(panel, LV_ALIGN_TOP_MID, 0, 120);
    lv_obj_set_style_radius(panel, 24, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xEAE1D4), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 18, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    g_ui.user_ta = lv_textarea_create(panel);
    lv_obj_set_size(g_ui.user_ta, 280, 46);
    lv_obj_align(g_ui.user_ta, LV_ALIGN_TOP_MID, 0, 20);
    lv_textarea_set_one_line(g_ui.user_ta, true);
    lv_textarea_set_placeholder_text(g_ui.user_ta, "用户名");
    lv_obj_set_style_text_font(g_ui.user_ta, ui_font_get_22(), 0);
    lv_obj_set_style_radius(g_ui.user_ta, 14, 0);
    lv_obj_set_style_border_width(g_ui.user_ta, 0, 0);
    lv_obj_set_style_pad_left(g_ui.user_ta, 12, 0);
    lv_obj_add_event_cb(g_ui.user_ta, on_ta_event, LV_EVENT_ALL, NULL);

    g_ui.pwd_ta = lv_textarea_create(panel);
    lv_obj_set_size(g_ui.pwd_ta, 280, 46);
    lv_obj_align_to(g_ui.pwd_ta, g_ui.user_ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_textarea_set_one_line(g_ui.pwd_ta, true);
    lv_textarea_set_password_mode(g_ui.pwd_ta, true);
    lv_textarea_set_placeholder_text(g_ui.pwd_ta, "密码");
    lv_obj_set_style_text_font(g_ui.pwd_ta, ui_font_get_22(), 0);
    lv_obj_set_style_radius(g_ui.pwd_ta, 14, 0);
    lv_obj_set_style_border_width(g_ui.pwd_ta, 0, 0);
    lv_obj_set_style_pad_left(g_ui.pwd_ta, 12, 0);
    lv_obj_add_event_cb(g_ui.pwd_ta, on_ta_event, LV_EVENT_ALL, NULL);

    // 登录按键
    lv_obj_t *login_btn = create_button(panel, "登录", on_login);
    lv_obj_set_size(login_btn, 120, 50);
    lv_obj_align(login_btn, LV_ALIGN_BOTTOM_LEFT, 32, -18);

    // 新增注册按键
    lv_obj_t *register_btn = create_button(panel, "注册", on_register);
    lv_obj_set_size(register_btn, 120, 50);
    lv_obj_align(register_btn, LV_ALIGN_BOTTOM_RIGHT, -32, -18);

    g_ui.msg_label = lv_label_create(g_ui.login_screen);
    lv_obj_set_style_text_color(g_ui.msg_label, lv_color_hex(0xA04C4C), 0);
    lv_obj_set_style_text_font(g_ui.msg_label, ui_font_get_22(), 0);
    lv_obj_align_to(g_ui.msg_label, panel, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);
    lv_obj_add_flag(g_ui.msg_label, LV_OBJ_FLAG_HIDDEN);

    g_ui.kb = lv_keyboard_create(g_ui.login_screen);
    lv_obj_set_size(g_ui.kb, lv_pct(100), 220);
    lv_obj_align(g_ui.kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(g_ui.kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(g_ui.kb, on_kb_event, LV_EVENT_ALL, NULL);
}

/* 主程序入口点 */
void app_ui_create(void)
{
    memset(&g_ui, 0, sizeof(g_ui));

    // 启动前先读取文件，初始化用户后端系统
    user_data_init();

    create_menu_screen();
    create_login_screen();

    lv_screen_load(g_ui.login_screen); // 默认进入登录画面
}

void app_ui_open_menu(void)
{
    if (g_ui.menu_screen == NULL)
    {
        memset(&g_ui, 0, sizeof(g_ui));
        user_data_init();
        create_menu_screen();
    }

    update_current_user_label();
    lv_screen_load(g_ui.menu_screen);
}
