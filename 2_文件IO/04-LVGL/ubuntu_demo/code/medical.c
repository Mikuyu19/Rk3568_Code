#include "medical.h"
#include "app_ui.h"
#include "ui_font.h"
#include "lvgl/lvgl.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define QUEUE_MAX_COUNT 10

typedef struct
{
    char room[16];
    char name[32];
    char title[32];
} doctor_info_t;

typedef struct
{
    int number;
    char name[32];
    char room[16];
} patient_info_t;

static lv_obj_t *medical_screen;
static lv_obj_t *call_box;
static lv_obj_t *current_call_label;
static lv_obj_t *doctor_name_label;
static lv_obj_t *doctor_title_label;
static lv_obj_t *doctor_avatar_label;
static lv_obj_t *queue_list;
static lv_obj_t *emergency_box;

static lv_obj_t *add_form_mask;
static lv_obj_t *add_form_name_ta;
static lv_obj_t *add_form_room_dd;
static lv_obj_t *add_form_kb;

static lv_timer_t *blink_timer;
static lv_timer_t *idle_timer;

static doctor_info_t doctors[2] = {
    {"1诊室", "王医生", "主任医师"},
    {"2诊室", "李医生", "副主任医师"}};

static patient_info_t current_patient = {12, "蔡徐坤", "1诊室"};

static patient_info_t wait_queue[QUEUE_MAX_COUNT] = {
    {13, "丁真", "1诊室"},
    {14, "马嘉祺", "1诊室"},
    {15, "刘耀文", "2诊室"},
    {16, "张真源", "1诊室"},
    {17, "宋亚轩", "2诊室"},
    {18, "严浩翔", "1诊室"}};

static int queue_count = 6;
static int next_number = 19;
static int blink_count;
static int blink_state;
static int idle_seconds;

static const char *name_pool[] = {
    "王源", "王俊凯", "易烊千玺", "大卫", "马云", "陈翔", "菲菲", "韩林"};

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

    if (blink_timer != NULL) // 如果闪烁定时器还在运行
    {
        lv_timer_del(blink_timer);
        blink_timer = NULL;
    }

    if (idle_timer != NULL) // 如果闲置定时器还在运行
    {
        lv_timer_del(idle_timer);
        idle_timer = NULL;
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
// 更新叫号信息
static void update_current_call(void)
{

    lv_label_set_text_fmt(
        current_call_label,
        "请 %03d 号 %s 到 %s",
        current_patient.number,
        current_patient.name,
        current_patient.room);
}
// 更新医生信息
static void update_doctor_info(void)
{
    int index = 0;

    if (strcmp(current_patient.room, "2诊室") == 0)
    {
        index = 1;
    }

    lv_label_set_text_fmt(doctor_name_label, "姓名：%s", doctors[index].name);
    lv_label_set_text_fmt(doctor_title_label, "职称：%s", doctors[index].title);
}
// 语音报号（模拟）
static void tts_speak(void)
{
    printf("TTS: 请 %03d 号 %s 到 %s\n",
           current_patient.number,
           current_patient.name,
           current_patient.room);
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

/* 创建并在屏幕中间弹出紧急公告 */
static void show_notice(const char *text)
{
    lv_obj_t *title_label;
    lv_obj_t *text_label;
    lv_obj_t *close_btn;

    close_notice();

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
}

static void close_notice(void)
{
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

/* 造一个假数据来填补队伍空缺。p 是传进来的目标结构体指针。 */
static void make_new_patient(patient_info_t *p)
{
    // 取模运算：利用当前的 next_number 算出一个 0~7 的数字（因为 name_pool 只有8个名字）
    int idx = (next_number - 19) % (int)(sizeof(name_pool) / sizeof(name_pool[0]));

    p->number = next_number++;

    snprintf(p->name, sizeof(p->name), "%s", name_pool[idx]);

    // 如果派的号码是偶数，余数是0，+1就是1诊室；奇数余数是1，+1就是2诊室。
    snprintf(p->room, sizeof(p->room), "%d诊室", (p->number % 2) + 1);
}

static void shift_queue_from(int index)
{
    // 如果没人在排队，或者给的 index 瞎写的（越界了），直接结束函数
    if (queue_count <= 0 || index < 0 || index >= queue_count)
    {
        return;
    }

    for (int i = index; i < queue_count - 1; i++)
    {
        wait_queue[i] = wait_queue[i + 1]; // 后一个人覆盖前一个人的数据
    }

    make_new_patient(&wait_queue[queue_count - 1]);
}

static void do_call_by_index(int index)
{
    if (queue_count <= 0 || index < 0 || index >= queue_count)
    {
        return;
    }

    current_patient = wait_queue[index];

    shift_queue_from(index);

    // 刷新
    update_current_call();
    update_wait_queue();
    update_doctor_info();

    start_blink(); // 屏幕闪起来
    tts_speak();   // 终端叫起来

    reset_idle(NULL);
}

/* 当我们在左边列表点了一下某个人时触发 */
static void on_queue_item(lv_event_t *e)
{
    // 这里的魔法在下面建列表的时候！我们把那一行的“序号 i” 强塞进了这个点击事件里。
    // 现在我们要把它强转回 int 类型取出来。
    int index = (int)(intptr_t)lv_event_get_user_data(e);

    // 取出行号后，直接调用总指挥函数叫他！
    do_call_by_index(index);
}

/* 核心 UI：根据最新的 wait_queue 数组，重绘左侧列表 */
static void update_wait_queue(void)
{
    lv_obj_clean(queue_list); // clean就是清场！把之前创建的一行行列表项全部干掉销毁，腾出空位。

    // 从 0 遍历到 queue_count (有几个人排队就跑几次循环)
    for (int i = 0; i < queue_count; i++)
    {
        // 先造个底板（用 button，因为按钮天生能点击响应，做列表项最好用）
        lv_obj_t *row = lv_button_create(queue_list);
        lv_obj_t *num_lab;
        lv_obj_t *name_lab;
        lv_obj_t *room_lab;
        char num_buf[16]; // 准备个小缓存装号码字符串

        // 把这行按钮的大小定死
        lv_obj_set_size(row, 390, 34);
        // 根据当前的 i 计算这行该放多低，i=0放在顶端，i=1往下挪 38 像素，形成排版
        lv_obj_set_pos(row, 0, i * 38);
        lv_obj_set_style_radius(row, 6, 0);                        // 给点小圆角
        lv_obj_set_style_bg_color(row, lv_color_hex(0xFFFFFF), 0); // 纯白底色
        lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(row, 1, 0);                      // 加 1 像素的边框线
        lv_obj_set_style_border_color(row, lv_color_hex(0xE5E5E5), 0); // 浅灰边框

        // 【极其重要】给这行按钮绑定上点击事件。
        // 第四个参数原本是要传指针的，我们把整数 i 强行伪装成了指针（通过 intptr_t）塞进去！
        // 这样点击时，系统就会带着 i 过去找 on_queue_item。
        lv_obj_add_event_cb(row, on_queue_item, LV_EVENT_CLICKED, (void *)(intptr_t)i);

        // --- 绘制本行的第一列：号码文字 ---
        snprintf(num_buf, sizeof(num_buf), "%03d", wait_queue[i].number); // 先把数字转成 00X 字符串
        num_lab = lv_label_create(row);                                   // 放在 row (按钮底板) 里面
        lv_label_set_text(num_lab, num_buf);
        lv_obj_set_style_text_font(num_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(num_lab, lv_color_black(), 0);
        lv_obj_align(num_lab, LV_ALIGN_LEFT_MID, 14, 0); // 靠左居中，向右挪 14 像素

        // --- 绘制本行的第二列：姓名文字 ---
        name_lab = lv_label_create(row);
        lv_label_set_text(name_lab, wait_queue[i].name); // 直接取数组里的名字
        lv_obj_set_style_text_font(name_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(name_lab, lv_color_black(), 0);
        lv_obj_align(name_lab, LV_ALIGN_LEFT_MID, 118, 0); // 靠左居中，向右挪 118 像素

        // --- 绘制本行的第三列：诊室文字 ---
        room_lab = lv_label_create(row);
        lv_label_set_text(room_lab, wait_queue[i].room); // 直接取数组里的诊室
        lv_obj_set_style_text_font(room_lab, ui_font_get_22(), 0);
        lv_obj_set_style_text_color(room_lab, lv_color_black(), 0);
        lv_obj_align(room_lab, LV_ALIGN_RIGHT_MID, -18, 0); // 靠右居中，向左缩 18 像素
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
    show_notice("系统维护中，请稍候...");
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

    if (queue_count >= QUEUE_MAX_COUNT)
    {
        show_popup("添加失败", "等待队列已满");
        return;
    }

    name = lv_textarea_get_text(add_form_name_ta);
    if (name == NULL || name[0] == '\0')
    {
        show_popup("添加失败", "请输入病人姓名");
        return;
    }
    lv_dropdown_get_selected_str(add_form_room_dd, room, sizeof(room));

    if (strcmp(room, "1") == 0)
    {
        snprintf(room, sizeof(room), "1诊室"); // C 语言格式化塞回去
    }
    else
    {
        snprintf(room, sizeof(room), "2诊室");
    }

    // 把名字拷贝进去
    snprintf(wait_queue[queue_count].name, sizeof(wait_queue[queue_count].name), "%s", name);
    // 把诊室拷贝进去
    snprintf(wait_queue[queue_count].room, sizeof(wait_queue[queue_count].room), "%s", room);
    // 把排号派给他，派完后号码递增备用
    wait_queue[queue_count].number = next_number++;
    // 队列人数 +1
    queue_count++;

    update_wait_queue();
    close_add_form();
    show_popup("添加成功", "病人已加入等待队列");
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
    lv_obj_set_size(panel, 420, 320);
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

    // 6. 造个真实的下拉框，给那两个诊室选项
    add_form_room_dd = lv_dropdown_create(panel);
    lv_obj_set_size(add_form_room_dd, 300, 46);

    lv_obj_align_to(add_form_room_dd, room_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_dropdown_set_options(add_form_room_dd, "1\n2");
    lv_obj_set_style_text_font(add_form_room_dd, ui_font_get_22(), 0);

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
    // 键盘也得有交互（打字、收起），绑上事件
    lv_obj_add_event_cb(add_form_kb, on_form_kb_event, LV_EVENT_ALL, NULL);

    reset_idle(NULL);
}

/* 提供给外部其它 C 文件的接口函数。
   比如后台有个线程收到了服务器发来的特殊字符串，想强行通知屏幕叫号，就调这个。
   假设 cmd 是 "@call,12,张三,1诊室"
*/
void medical_parse_cmd(const char *cmd)
{
    char buf[128];         // 因为进来的 cmd 是 const (不可改的)，弄个草稿纸 buf 用来切蛋糕
    char *token;           // 刀子：指着切下来的每一块蛋糕
    char *save_ptr = NULL; // 记忆点：告诉刀子上次切到哪了 (strtok_r 需要)
    char number_str[16];   // 装第二刀切下的号码
    char name[32];         // 装第三刀切下的名字
    char room[16];         // 装第四刀切下的诊室
    int queue_index = -1;  // 用来找人，默认 -1 没找到

    if (cmd == NULL)
    {
        return; // 空指令不理
    }

    snprintf(buf, sizeof(buf), "%s", cmd); // 把原话原原本本抄到草稿纸上

    // 第一刀：找到第一个逗号，把前面的字切下来赋给 token
    token = strtok_r(buf, ",", &save_ptr);
    // 判断切下来的是不是我们约定的暗号 "@call"，不是就丢掉不玩了
    if (token == NULL || strcmp(token, "@call") != 0)
    {
        return;
    }

    // 第二刀：继续切，提取号码字符
    token = strtok_r(NULL, ",", &save_ptr);
    if (token == NULL)
        return;
    snprintf(number_str, sizeof(number_str), "%s", token);

    // 第三刀：继续切，提取姓名
    token = strtok_r(NULL, ",", &save_ptr);
    if (token == NULL)
        return;
    snprintf(name, sizeof(name), "%s", token);

    // 第四刀：继续切，提取诊室
    token = strtok_r(NULL, ",", &save_ptr);
    if (token == NULL)
        return;
    // 容错处理：有可能只发过来 "1"，那就帮他拼成 "1诊室"
    if (strstr(token, "诊室") == NULL)
    {
        snprintf(room, sizeof(room), "%s诊室", token);
    }
    else // 已经带诊室了，直接存
    {
        snprintf(room, sizeof(room), "%s", token);
    }

    // 所有的参数成功解包了，现在强行篡改正在叫号的位置！
    current_patient.number = atoi(number_str); // atoi 把字符串的 "12" 强转成整数 12
    snprintf(current_patient.name, sizeof(current_patient.name), "%s", name);
    snprintf(current_patient.room, sizeof(current_patient.room), "%s", room);

    // 有个问题：万一叫的这个张三，他已经在咱们的排队列表里了，他走了列表不就该空一位吗？
    // 找一找他！
    for (int i = 0; i < queue_count; i++)
    {
        if (wait_queue[i].number == current_patient.number) // 只要号码对上了
        {
            queue_index = i; // 记录下这个张三原来排在第几名
            break;           // 找到了就不用往后搜了
        }
    }

    // 如果真找到了（不是 -1），就把他从排队列表里拿掉，让后面的人往前挪
    if (queue_index >= 0)
    {
        shift_queue_from(queue_index);
    }

    // 更新画面和声光系统！
    update_current_call();
    update_wait_queue();
    update_doctor_info();

    start_blink();
    tts_speak();
    reset_idle(NULL);
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
    lv_label_set_text(lab, "返回菜单");
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

    // UI 都画完了，最后再启动负责统计无操作秒数的后台定时器，定为 1000ms 跑一次 idle_timer_cb
    idle_timer = lv_timer_create(idle_timer_cb, 1000, NULL);

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