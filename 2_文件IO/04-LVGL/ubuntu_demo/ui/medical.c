#include "medical.h"
#include "app_ui.h"
#include "ui_font.h"
#include "lvgl/lvgl.h"
#include "../core/app_state.h" // 共享状态：网络线程写、LVGL 定时器读

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// ======================== LVGL 控件指针（UI 状态，不属于数据层） ========================

static lv_obj_t *medical_screen;             // 医疗叫号根屏幕
static lv_obj_t *call_box;                   // 顶部叫号大卡片
static lv_obj_t *current_call_label;         // "请 012 号 张三 到 1诊室"
static lv_obj_t *doctor_name_label;          // 医生姓名
static lv_obj_t *doctor_title_label;         // 医生职称
static lv_obj_t *doctor_avatar_label;        // 头像里的"医"字
static lv_obj_t *queue_list;                 // 排队列表容器
static lv_obj_t *emergency_box;              // 公告弹窗指针
static lv_obj_t *add_form_pinyin_ime = NULL; // 拼音输入法候选框

static lv_obj_t *add_form_mask;      // 添加病人黑幕
static lv_obj_t *add_form_name_ta;   // 姓名输入框
static lv_obj_t *add_form_room_dd;   // 诊室下拉框
static lv_obj_t *add_form_kb;        // 软键盘
static lv_obj_t *add_form_urgent_sw; // 急诊插队开关

// ======================== LVGL 定时器 ========================

static lv_timer_t *blink_timer;  // 叫号闪烁动画
static lv_timer_t *idle_timer;   // 闲置计时器（30 秒自动关公告）
static lv_timer_t *poll_timer;   // 网络事件轮询（200ms 检查 app_state 标志）
static lv_timer_t *notice_timer; // 公告按 duration 自动关闭

// ======================== 动画/闲置状态 ========================

static int blink_count;  // 闪烁次数
static int blink_state;  // 当前亮灭
static int idle_seconds; // 闲置秒数计数

// ======================== 前向声明 ========================

static void close_notice(void);
static void close_add_form(void);
static void update_wait_queue(void);

// 闲置时间清零函数
static void reset_idle(lv_event_t *e)
{
    LV_UNUSED(e);
    idle_seconds = 0; // 闲置秒数归 0
}

// 弹窗上面的“确定”按钮的点击事件
static void popup_close_cb(lv_event_t *e)
{
    lv_obj_t *mbox = lv_event_get_user_data(e);
    lv_msgbox_close(mbox);
}

// 封装好的通用小弹窗工具函数，想弹什么字直接调用就行
static void show_popup(const char *title, const char *text)
{
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_obj_t *title_label = lv_msgbox_add_title(mbox, title);
    lv_obj_t *text_label = lv_msgbox_add_text(mbox, text);
    lv_obj_t *ok_btn = lv_msgbox_add_footer_button(mbox, "确定");

    // 把标题、正文、按钮上的字都设置成 22 号自定义字体
    lv_obj_set_style_text_font(title_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(text_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(ok_btn, ui_font_get_22(), 0);

    // 给“确定”按钮上个浅蓝色的背景，文字设为黑色
    lv_obj_set_style_bg_color(ok_btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_bg_opa(ok_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(ok_btn, lv_color_black(), 0);

    lv_obj_add_event_cb(ok_btn, popup_close_cb, LV_EVENT_CLICKED, mbox);

    lv_obj_center(mbox);
}

// 页面退出、销毁时的清理工作
static void medical_delete_cb(lv_event_t *e)
{
    LV_UNUSED(e);

    if (blink_timer != NULL)
    {
        lv_timer_del(blink_timer);
        blink_timer = NULL;
    }

    if (idle_timer != NULL)
    {
        lv_timer_del(idle_timer);
        idle_timer = NULL;
    }

    if (poll_timer != NULL)
    {
        lv_timer_del(poll_timer);
        poll_timer = NULL;
    }

    // 把所有跟这个页面相关的全局控件指针全设为 NULL
    emergency_box = NULL;
    add_form_mask = NULL;
    add_form_name_ta = NULL;
    add_form_room_dd = NULL;
    add_form_kb = NULL;
    queue_list = NULL;
    medical_screen = NULL;
}
// 更新叫号大文字
// 数据来源：app_state（可能被本地触屏、也可能被网络线程更新）
static void update_current_call(void)
{
    patient_info_t p;
    doctor_info_t d;
    app_state_get_call(&p, &d); // 线程安全读取

    lv_label_set_text_fmt(
        current_call_label,
        "请 %03d 号 %s 到 %s",
        p.number, p.name, p.room);
}

// 更新医生信息区域
static void update_doctor_info(void)
{
    patient_info_t p;
    doctor_info_t d;
    app_state_get_call(&p, &d); // 一次调用同时拿到病人 + 自动匹配的医生

    lv_label_set_text_fmt(doctor_name_label, "姓名：%s", d.name);
    lv_label_set_text_fmt(doctor_title_label, "职称：%s", d.title);
}

// 语音报号（模拟 TTS 语音模块）
// 真实板子上应改为 write() 向串口发送文本
static void tts_speak(void)
{
    patient_info_t p;
    doctor_info_t d;
    app_state_get_call(&p, &d);

    printf("TTS: 请 %03d 号 %s 到 %s\n", p.number, p.name, p.room);
}

// 控制背景闪烁的定时器回调任务
static void blink_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if (blink_state) // 如果当前状态是 1 (高亮状态)
    {
        lv_obj_set_style_bg_color(call_box, lv_color_hex(0xFFFFFF), 0);
    }
    else
    {
        lv_obj_set_style_bg_color(call_box, lv_color_hex(0xFFE4E1), 0); // 浅红色
    }

    blink_state = !blink_state; // 逻辑取反
    blink_count++;

    if (blink_count >= 6) // 如果已经闪了 6 次了（大概 1.5 秒）
    {
        lv_obj_set_style_bg_color(call_box, lv_color_hex(0xFFFFFF), 0); // 确保最后停在白色
        lv_timer_del(blink_timer);                                      // 任务完成，自毁定时器
        blink_timer = NULL;                                             // 指针清空
        blink_count = 0;                                                // 状态重置
        blink_state = 0;
    }
}

/* 触发顶部背景闪烁的启动开关 */
static void start_blink(void)
{
    if (blink_timer != NULL) // 如果系统里已经有个闪烁定时器在跑了
    {
        lv_timer_del(blink_timer);
    }

    blink_count = 0;
    blink_state = 0;
    // 创建一个定时器，规定每隔 250ms，就去执行一次 blink_timer_cb 函数
    blink_timer = lv_timer_create(blink_timer_cb, 250, NULL);
}

/* 点击了紧急公告弹窗里的“关闭”按钮 */
static void notice_close_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    close_notice();
    reset_idle(NULL); // 有人按了屏幕，所以重置闲置计时
}

static void notice_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    notice_timer = NULL;
    close_notice();
}

/* 创建并在屏幕中间弹出紧急公告 */
static void show_notice(const char *text, int duration_sec)
{
    lv_obj_t *title_label;
    lv_obj_t *text_label;
    lv_obj_t *close_btn;

    close_notice();
    idle_seconds = 0;

    emergency_box = lv_msgbox_create(NULL);
    title_label = lv_msgbox_add_title(emergency_box, "紧急公告");
    text_label = lv_msgbox_add_text(emergency_box, text);
    close_btn = lv_msgbox_add_footer_button(emergency_box, "关闭");

    lv_obj_set_style_text_font(title_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(text_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_font(close_btn, ui_font_get_22(), 0);

    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_bg_opa(close_btn, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(close_btn, lv_color_black(), 0);

    // 把 notice_close_cb 绑定给按钮
    lv_obj_add_event_cb(close_btn, notice_close_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_center(emergency_box); // 居中

    if (duration_sec > 0)
    {
        notice_timer = lv_timer_create(notice_timer_cb, duration_sec * 1000, NULL);
        lv_timer_set_repeat_count(notice_timer, 1);
    }
}

static void close_notice(void)
{
    if (notice_timer != NULL)
    {
        lv_timer_del(notice_timer);
        notice_timer = NULL;
    }

    if (emergency_box != NULL)
    {
        lv_msgbox_close(emergency_box);
        emergency_box = NULL;
    }
}

/* 统计闲置秒数的定时器回调（每 1000ms = 1秒 被执行一次） */
static void idle_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    idle_seconds++;

    if (idle_seconds >= 30)
    {
        close_notice();
        idle_seconds = 0;
    }
}

// 叫号核心：弹出队列第 index 个人，成为当前叫号
// 数据操作完全委托给 app_state（内部加锁保证线程安全）
static void do_call_by_index(int index)
{
    if (app_state_call_by_index(index) != 0)
    {
        return; // 越界或队列为空，静默忽略
    }

    // 刷新界面
    update_current_call();
    update_wait_queue();
    update_doctor_info();

    start_blink(); // 背景闪烁动画
    tts_speak();   // TTS 语音播报

    reset_idle(NULL);
}

/* 当我们在左边列表点了一下某个人时触发 */
static void on_queue_item(lv_event_t *e)
{
    // 把那一行的“序号 i” 强塞进了这个点击事件里。
    // 把它强转回 int 类型取出来。
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    // 取出行号后，直接调用总指挥函数叫他！
    do_call_by_index(index);
}

// 根据 app_state 中的候诊队列重绘左侧列表
static void update_wait_queue(void)
{
    lv_obj_clean(queue_list); // 清空旧列表项

    int count = app_state_get_queue_count();

    for (int i = 0; i < count; i++)
    {
        patient_info_t p;
        app_state_get_queue_item(i, &p); // 线程安全读取第 i 个病人

        // 底板按钮（整个列表行）
        lv_obj_t *row = lv_button_create(queue_list);
        lv_obj_set_size(row, 390, 34);
        lv_obj_set_pos(row, 0, i * 38);
        lv_obj_set_style_radius(row, 6, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 1, 0);
        lv_obj_set_style_border_color(row, lv_color_hex(0xE5E5E5), 0);
        lv_obj_add_event_cb(row, on_queue_item, LV_EVENT_CLICKED,
                            (void *)(intptr_t)i);

        // 第一列：号码
        char num_buf[16];
        snprintf(num_buf, sizeof(num_buf), "%03d", p.number);
        lv_obj_t *num_lab = lv_label_create(row);
        lv_label_set_text(num_lab, num_buf);
        lv_obj_set_style_text_font(num_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(num_lab, lv_color_black(), 0);
        lv_obj_align(num_lab, LV_ALIGN_LEFT_MID, 14, 0);

        // 第二列：姓名
        lv_obj_t *name_lab = lv_label_create(row);
        lv_label_set_text(name_lab, p.name);
        lv_obj_set_style_text_font(name_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(name_lab, lv_color_black(), 0);
        lv_obj_align(name_lab, LV_ALIGN_LEFT_MID, 118, 0);

        // 第三列：诊室
        lv_obj_t *room_lab = lv_label_create(row);
        lv_label_set_text(room_lab, p.room);
        lv_obj_set_style_text_font(room_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(room_lab, lv_color_black(), 0);
        lv_obj_align(room_lab, LV_ALIGN_RIGHT_MID, -18, 0);
    }
}

// 左下角模拟叫号
static void on_simulate_call(lv_event_t *e)
{
    LV_UNUSED(e);
    do_call_by_index(0); // 直接叫队列里的第 0 个人（排在最前面的）
}

// 底部中间紧急公告按钮点下去了
static void on_notice_btn(lv_event_t *e)
{
    LV_UNUSED(e);
    show_notice("系统维护中，请稍候...", 0);
    reset_idle(NULL);
}

// 顶部返回按钮点下去了
static void on_back_menu(lv_event_t *e)
{
    LV_UNUSED(e);
    close_add_form();
    close_notice();
    app_ui_open_menu();
}

// 收起添加病人表单底部的软键盘
static void hide_form_keyboard(void)
{
    if (add_form_kb == NULL)
    {
        return;
    }
    lv_keyboard_set_textarea(add_form_kb, NULL);
    lv_obj_add_flag(add_form_kb, LV_OBJ_FLAG_HIDDEN);
}
// 删除黑幕
static void close_add_form(void)
{
    if (add_form_mask != NULL)
    {
        lv_obj_delete(add_form_mask); // delete会把它，以及它上面长出的所有输入框、按钮连带全干掉！
        // 因为被 LVGL 内部销毁了，但 C 语言这边的指针变量里面存的地址还是旧的（野指针）
        // 必须立刻全部置为 NULL，防暴毙！
        add_form_mask = NULL;
        add_form_name_ta = NULL;
        add_form_room_dd = NULL;
        add_form_kb = NULL;

        add_form_pinyin_ime = NULL;
        add_form_urgent_sw = NULL;
    }
}

// 当你在弹窗里的输入框 (textarea) 上点了一下
static void on_form_ta_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED)
    {
        lv_keyboard_set_textarea(add_form_kb, ta);
        lv_obj_clear_flag(add_form_kb, LV_OBJ_FLAG_HIDDEN); // 让键盘浮现
    }
}

static void on_form_kb_event(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        hide_form_keyboard();
    }
}

static void on_add_form_cancel(lv_event_t *e)
{
    LV_UNUSED(e);
    close_add_form();
    reset_idle(NULL);
}

static void on_add_form_ok(lv_event_t *e)
{
    char room[16];
    const char *name;

    LV_UNUSED(e);

    // 检查队列是否已满
    if (app_state_get_queue_count() >= QUEUE_MAX_COUNT)
    {
        show_popup("添加失败", "等待队列已满");
        return;
    }

    // 校验姓名不为空
    name = lv_textarea_get_text(add_form_name_ta);
    if (name == NULL || name[0] == '\0')
    {
        show_popup("添加失败", "请输入病人姓名");
        return;
    }

    // 读取下拉框选择的诊室
    lv_dropdown_get_selected_str(add_form_room_dd, room, sizeof(room));
    if (strcmp(room, "1") == 0)
    {
        snprintf(room, sizeof(room), "1诊室");
    }
    else
    {
        snprintf(room, sizeof(room), "2诊室");
    }

    // 读取急诊插队开关状态
    bool is_urgent = lv_obj_has_state(add_form_urgent_sw, LV_STATE_CHECKED);

    // 委托给 app_state（内部加锁、分配号码、处理插队算法）
    app_state_add_patient(name, room, is_urgent ? 1 : 0);

    // 刷新列表 UI
    update_wait_queue();
    close_add_form();
    show_popup("添加成功", is_urgent ? "急诊病人已插队至最前！" : "病人已加入等待队列");

    reset_idle(NULL);
}

// 添加病人UI
static void on_add_patient(lv_event_t *e)
{
    lv_obj_t *panel;
    lv_obj_t *title;
    lv_obj_t *btn;
    lv_obj_t *lab;
    lv_obj_t *room_label;

    LV_UNUSED(e);

    if (add_form_mask != NULL)
    {
        return;
    }

    // 1. 先造一张黑布（蒙版），盖在整个医疗屏幕最上层
    add_form_mask = lv_obj_create(medical_screen);
    lv_obj_set_size(add_form_mask, 1024, 600);                           // 盖满屏幕
    lv_obj_set_style_bg_color(add_form_mask, lv_color_hex(0x000000), 0); // 纯黑
    lv_obj_set_style_bg_opa(add_form_mask, LV_OPA_50, 0);
    lv_obj_set_style_border_width(add_form_mask, 0, 0);
    lv_obj_clear_flag(add_form_mask, LV_OBJ_FLAG_SCROLLABLE);

    // 2. 在这块黑布的中间，放一块用来当弹窗的白色面板
    panel = lv_obj_create(add_form_mask);
    lv_obj_set_size(panel, 420, 380);

    lv_obj_center(panel);
    lv_obj_set_style_radius(panel, 18, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_pad_all(panel, 18, 0);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLLABLE);

    // 3. 在白板最上面写个标题
    title = lv_label_create(panel);
    lv_label_set_text(title, "添加病人");
    lv_obj_set_style_text_font(title, ui_font_get_26(), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    // 4. 造一个让人打字的文本输入框
    add_form_name_ta = lv_textarea_create(panel);
    lv_obj_set_size(add_form_name_ta, 300, 46);
    lv_obj_align(add_form_name_ta, LV_ALIGN_TOP_MID, 0, 56);
    lv_textarea_set_one_line(add_form_name_ta, true);
    lv_textarea_set_placeholder_text(add_form_name_ta, "姓名");
    lv_obj_set_style_text_font(add_form_name_ta, ui_font_get_22(), 0);
    // 把输入框被点击时的事件，绑定给 on_form_ta_event
    lv_obj_add_event_cb(add_form_name_ta, on_form_ta_event, LV_EVENT_ALL, NULL);

    // 5. 接下来要做个选诊室的，先写个提示文本
    room_label = lv_label_create(panel);
    lv_label_set_text(room_label, "选择诊室");
    lv_obj_set_style_text_font(room_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(room_label, lv_color_hex(0x5E5248), 0);
    // 这个牛逼：对齐目标变成上面那个输入框(add_form_name_ta)，在它正下方偏左对齐往下走 18px
    lv_obj_align_to(room_label, add_form_name_ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 18);

    // 6. 造个下拉框，给那两个诊室选项
    add_form_room_dd = lv_dropdown_create(panel);
    lv_obj_set_size(add_form_room_dd, 300, 46);

    lv_obj_align_to(add_form_room_dd, room_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_dropdown_set_options(add_form_room_dd, "1\n2");
    lv_obj_set_style_text_font(add_form_room_dd, ui_font_get_22(), 0);

    // 急诊插队开关选项
    lv_obj_t *urgent_label = lv_label_create(panel);
    lv_label_set_text(urgent_label, "开启急诊插队");
    lv_obj_set_style_text_font(urgent_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(urgent_label, lv_color_hex(0xC0392B), 0); // 红色提示
    lv_obj_align_to(urgent_label, add_form_room_dd, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    add_form_urgent_sw = lv_switch_create(panel);
    lv_obj_align_to(add_form_urgent_sw, urgent_label, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    // 7. 造白板左下角的【确定】按钮
    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 120, 48);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 40, 0); // 靠左下
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    // 绑定写入数据的核心函数！
    lv_obj_add_event_cb(btn, on_add_form_ok, LV_EVENT_CLICKED, NULL);
    lab = lv_label_create(btn);
    lv_label_set_text(lab, "确定");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // 8. 造白板右下角的【取消】按钮
    btn = lv_button_create(panel);
    lv_obj_set_size(btn, 120, 48);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -40, 0); // 靠右下
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    // 绑定啥都不干直接关闭销毁的函数
    lv_obj_add_event_cb(btn, on_add_form_cancel, LV_EVENT_CLICKED, NULL);
    lab = lv_label_create(btn);
    lv_label_set_text(lab, "取消");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // 9. 所有的输入和按钮搞定了，最后在全屏幕最下面塞一个软键盘
    add_form_kb = lv_keyboard_create(add_form_mask);
    lv_obj_set_size(add_form_kb, lv_pct(100), 220);
    lv_obj_align(add_form_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(add_form_kb, LV_OBJ_FLAG_HIDDEN);
    lv_keyboard_set_mode(add_form_kb, LV_KEYBOARD_MODE_TEXT_LOWER); // 小写模式，拼音26键输入
    // 键盘也得有交互（打字、收起），绑上事件
    lv_obj_add_event_cb(add_form_kb, on_form_kb_event, LV_EVENT_ALL, NULL);

    // 新增拼音输入法
    add_form_pinyin_ime = lv_ime_pinyin_create(add_form_mask);
    // 设置拼音中字体的显示和大小
    lv_obj_set_style_text_font(add_form_pinyin_ime, ui_font_get_22(), 0);
    // 实体键盘绑定事件
    lv_ime_pinyin_set_keyboard(add_form_pinyin_ime, add_form_kb);
    lv_obj_align_to(add_form_pinyin_ime, add_form_kb, LV_ALIGN_OUT_TOP_MID, 0, -2);
    reset_idle(NULL);
}

// ========================
// 网络事件轮询定时器（200ms 间隔）
// ========================
//
// 这是连接"网络线程"和"UI 线程"的桥梁：
//   网络线程 → app_state_set_xxx() 写数据 + 置标志位
//   lv_timer 200ms 定时 → app_state_has_new_xxx() 检测标志 → 刷新界面
//
// 为什么用轮询而不是网络线程直接调 LVGL？
//   LVGL 不是线程安全的，网络线程直接操作控件会导致崩溃。
//   轮询方式只在 LVGL 主线程中操作控件，安全可靠。

static void poll_timer_cb(lv_timer_t *timer)
{
    (void)timer;

    // 1. 检查是否有新叫号
    if (app_state_has_new_call())
    {
        update_current_call();
        update_wait_queue(); // 队列可能也变了：被叫到的病人已移除
        update_doctor_info();
        start_blink();
        tts_speak();
        reset_idle(NULL);
    }

    // 2. 检查候诊队列是否有变化
    if (app_state_has_new_queue())
    {
        update_wait_queue();
    }

    // 3. 检查医生信息是否有更新
    if (app_state_has_new_doctor())
    {
        update_doctor_info();
    }

    // 4. 检查是否有新公告要弹出
    if (app_state_has_new_notice())
    {
        char text[256];
        int duration;
        app_state_get_notice(text, sizeof(text), &duration);
        show_notice(text, duration);
    }

    // 5. 检查是否需要关闭公告
    if (app_state_has_clear_notice())
    {
        close_notice();
    }
}

lv_obj_t *medical_create_screen(lv_event_cb_t back_cb)
{
    // 一堆局部指针，用来在下面建对象临时挂钩子用
    lv_obj_t *queue_box;
    lv_obj_t *doctor_box;
    lv_obj_t *btn_box;
    lv_obj_t *title;
    lv_obj_t *queue_title;
    lv_obj_t *doctor_title;
    lv_obj_t *avatar_box;
    lv_obj_t *header_row;
    lv_obj_t *lab;
    lv_obj_t *btn;

    medical_screen = lv_obj_create(NULL);
    lv_obj_set_size(medical_screen, 1024, 600);
    lv_obj_set_style_bg_color(medical_screen, lv_color_hex(0xF0F4F7), 0);
    lv_obj_clear_flag(medical_screen, LV_OBJ_FLAG_SCROLLABLE);
    // 这句话意思是，当这个屏幕被摧毁时，顺带去执行 medical_delete_cb 扫尾
    lv_obj_add_event_cb(medical_screen, medical_delete_cb, LV_EVENT_DELETE, NULL);
    // 给整个背景贴个保护膜：任何按压事件，都会唤醒 reset_idle 把挂机倒计时清零
    lv_obj_add_event_cb(medical_screen, reset_idle, LV_EVENT_PRESSED, NULL);

    // --- 1. 顶部大卡片（挂在根屏幕最上） ---
    call_box = lv_obj_create(medical_screen);
    lv_obj_set_size(call_box, 960, 150);
    lv_obj_align(call_box, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_bg_color(call_box, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(call_box, 16, 0);
    lv_obj_set_style_border_width(call_box, 0, 0);
    lv_obj_remove_flag(call_box, LV_OBJ_FLAG_SCROLLABLE);

    // "当前叫号" 这几个字
    title = lv_label_create(call_box);
    lv_label_set_text(title, "当前叫号");
    lv_obj_set_style_text_font(title, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x5E5248), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 20, 12);

    // 返回按钮
    btn = lv_button_create(call_box);
    lv_obj_set_size(btn, 120, 40);
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -18, 10);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    // 【重要】当它被点击，去执行从 app_ui.c 里传过来的 back_cb 函数（退回主界面）
    lv_obj_add_event_cb(btn, back_cb, LV_EVENT_CLICKED, NULL);
    lab = lv_label_create(btn);

#if 0
    lv_label_set_text(lab, "返回菜单");
#endif

#if 1
    lv_label_set_text(lab, "退出登录");
#endif

    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // 巨型大字牌（等会这里面的文字会被 update_current_call 不断覆盖重写）
    current_call_label = lv_label_create(call_box);
    lv_obj_set_width(current_call_label, 900);
    lv_obj_set_style_text_font(current_call_label, ui_font_get_48(), 0);        // 48号超大字
    lv_obj_set_style_text_color(current_call_label, lv_color_hex(0xC0392B), 0); // 深红色，显眼！
    lv_obj_set_style_text_align(current_call_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(current_call_label, LV_ALIGN_CENTER, 0, 12);

    // --- 2. 左侧模块（排队列表外壳） ---
    queue_box = lv_obj_create(medical_screen);
    lv_obj_set_size(queue_box, 460, 260); // 占屏幕左边一半偏下
    lv_obj_align(queue_box, LV_ALIGN_LEFT_MID, 30, 30);
    lv_obj_set_style_bg_color(queue_box, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(queue_box, 16, 0);
    lv_obj_set_style_border_width(queue_box, 0, 0);
    lv_obj_remove_flag(queue_box, LV_OBJ_FLAG_SCROLLABLE);

    // 标题文本
    queue_title = lv_label_create(queue_box);
    lv_label_set_text(queue_title, "等候队列");
    lv_obj_set_style_text_font(queue_title, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(queue_title, lv_color_hex(0x5E5248), 0);
    lv_obj_align(queue_title, LV_ALIGN_TOP_LEFT, 18, 12);

    // 队列列表那三个“列名表头”，必须是静止不动的
    header_row = lv_obj_create(queue_box);
    lv_obj_set_size(header_row, 410, 34);
    lv_obj_align(header_row, LV_ALIGN_TOP_MID, 0, 46);                // 挂在标题下
    lv_obj_set_style_bg_color(header_row, lv_color_hex(0xEAE1D4), 0); // 淡淡的茶色
    lv_obj_set_style_border_width(header_row, 0, 0);
    lv_obj_set_style_radius(header_row, 8, 0);
    lv_obj_remove_flag(header_row, LV_OBJ_FLAG_SCROLLABLE);

    lab = lv_label_create(header_row);
    lv_label_set_text(lab, "号码");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_align(lab, LV_ALIGN_LEFT_MID, 14, 0);

    lab = lv_label_create(header_row);
    lv_label_set_text(lab, "姓名");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_align(lab, LV_ALIGN_LEFT_MID, 118, 0);

    lab = lv_label_create(header_row);
    lv_label_set_text(lab, "诊室");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_align(lab, LV_ALIGN_RIGHT_MID, -18, 0);

    // 【核心】建一个透明的、只有滚动条属性的容器，等下 update_wait_queue 里建的一行行列表就是塞进这里
    queue_list = lv_obj_create(queue_box);
    lv_obj_set_size(queue_list, 410, 150); // 留出上面表头的高度
    lv_obj_align(queue_list, LV_ALIGN_TOP_MID, 0, 88);
    lv_obj_set_style_bg_opa(queue_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(queue_list, 0, 0);
    lv_obj_set_style_pad_all(queue_list, 0, 0);
    lv_obj_set_scroll_dir(queue_list, LV_DIR_VER);

    // --- 3. 右侧模块（显示医生的白板外壳） ---
    doctor_box = lv_obj_create(medical_screen);
    lv_obj_set_size(doctor_box, 460, 260);                 // 也是 460x260 大小
    lv_obj_align(doctor_box, LV_ALIGN_RIGHT_MID, -30, 30); // 靠右侧放
    lv_obj_set_style_bg_color(doctor_box, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(doctor_box, 16, 0);
    lv_obj_set_style_border_width(doctor_box, 0, 0);
    lv_obj_remove_flag(doctor_box, LV_OBJ_FLAG_SCROLLABLE);

    // 写个标题
    doctor_title = lv_label_create(doctor_box);
    lv_label_set_text(doctor_title, "医生信息");
    lv_obj_set_style_text_font(doctor_title, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(doctor_title, lv_color_hex(0x5E5248), 0);
    lv_obj_align(doctor_title, LV_ALIGN_TOP_LEFT, 18, 12);

    // 造一个正方形，利用巨大的圆角 55 将其切成一个正圆，当做假头像
    avatar_box = lv_obj_create(doctor_box);
    lv_obj_set_size(avatar_box, 110, 110);
    lv_obj_align(avatar_box, LV_ALIGN_LEFT_MID, 30, 20);
    lv_obj_set_style_radius(avatar_box, 55, 0); // 半径是宽度一半 = 正圆
    lv_obj_set_style_bg_color(avatar_box, lv_color_hex(0xAEEEEE), 0);
    lv_obj_set_style_border_width(avatar_box, 0, 0);
    lv_obj_remove_flag(avatar_box, LV_OBJ_FLAG_SCROLLABLE);

    // 在正圆形盒子里居中写个 “医” 字
    doctor_avatar_label = lv_label_create(avatar_box);
    lv_label_set_text(doctor_avatar_label, "医");
    lv_obj_set_style_text_font(doctor_avatar_label, ui_font_get_48(), 0);
    lv_obj_set_style_text_color(doctor_avatar_label, lv_color_hex(0x2C3E50), 0);
    lv_obj_center(doctor_avatar_label);

    // 预留给 update_doctor_info 刷新覆盖的空白文本框
    doctor_name_label = lv_label_create(doctor_box);
    lv_obj_set_style_text_font(doctor_name_label, ui_font_get_26(), 0);
    lv_obj_set_style_text_color(doctor_name_label, lv_color_hex(0x333333), 0);
    lv_obj_align(doctor_name_label, LV_ALIGN_LEFT_MID, 180, -10); // 在头像右边偏上

    doctor_title_label = lv_label_create(doctor_box);
    lv_obj_set_style_text_font(doctor_title_label, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(doctor_title_label, lv_color_hex(0x666666), 0);
    lv_obj_align(doctor_title_label, LV_ALIGN_LEFT_MID, 180, 36); // 在头像右边偏下

    // --- 4. 底部大通条模块（放三个横向大控制按钮的地方） ---
    btn_box = lv_obj_create(medical_screen);
    lv_obj_set_size(btn_box, 960, 90); // 也是 960 宽，压底
    lv_obj_align(btn_box, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_bg_color(btn_box, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_radius(btn_box, 16, 0);
    lv_obj_set_style_border_width(btn_box, 0, 0);
    lv_obj_remove_flag(btn_box, LV_OBJ_FLAG_SCROLLABLE);

    // 第一个按键：【模拟叫号】 (挂在底栏靠左)
    btn = lv_button_create(btn_box);
    lv_obj_set_size(btn, 180, 52);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 80, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    // 绑定具体的函数
    lv_obj_add_event_cb(btn, on_simulate_call, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, reset_idle, LV_EVENT_CLICKED, NULL); // 同时兼具重置倒计时功能
    lab = lv_label_create(btn);
    lv_label_set_text(lab, "模拟叫号");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // 第二个按键：【紧急公告】 (挂在底栏正中间)
    btn = lv_button_create(btn_box);
    lv_obj_set_size(btn, 180, 52);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_add_event_cb(btn, on_notice_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, reset_idle, LV_EVENT_CLICKED, NULL);
    lab = lv_label_create(btn);
    lv_label_set_text(lab, "紧急公告");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // 第三个按键：【添加病人】 (挂在底栏靠右)
    btn = lv_button_create(btn_box);
    lv_obj_set_size(btn, 180, 52);
    lv_obj_align(btn, LV_ALIGN_RIGHT_MID, -80, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xAEEEEE), 0);
    lv_obj_add_event_cb(btn, on_add_patient, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn, reset_idle, LV_EVENT_CLICKED, NULL);
    lab = lv_label_create(btn);
    lv_label_set_text(lab, "添加病人");
    lv_obj_set_style_text_font(lab, ui_font_get_22(), 0);
    lv_obj_set_style_text_color(lab, lv_color_black(), 0);
    lv_obj_center(lab);

    // --- 所有 UI 零件组装完成！调用那三个 update 接口，把刚才填进去的那些数组假数据渲染上墙 ---
    update_current_call();
    update_wait_queue();
    update_doctor_info();

    // 闲置计时器：每 1 秒检查，30 秒无人操作自动关公告
    idle_timer = lv_timer_create(idle_timer_cb, 1000, NULL);

    // 网络轮询定时器：每 200ms 检查 app_state 标志位，有新数据就刷新界面
    poll_timer = lv_timer_create(poll_timer_cb, 200, NULL);

    // 返回造好的根屏幕，让外部接收
    return medical_screen;
}

/* 外部 main 或者 app_ui 里调用的初始化入口点 */
void medical_init(void)
{
    // 调用大工程搭建屏幕，同时把 on_back_menu（如果外面没写死）塞进去
    medical_create_screen(on_back_menu);

    // lv_screen_load 负责完成最终的一击：把当前活跃展示的屏幕切成咱们刚才画的医疗大屏！
    lv_screen_load(medical_screen);
}
