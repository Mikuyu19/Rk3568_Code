#include "album.h"
#include "ui_font.h" // 引入自定义中文字体模块

#include <limits.h> // 引入 PATH_MAX
#include <stdio.h>  // 引入 snprintf
#include <string.h> // 引入 memset, memcpy

#define ALBUM_IMAGE_COUNT 3 // 定义相册系统中一共包含 3 张图片

/* 固定的图片文件名列表 */
static const char *g_album_files[ALBUM_IMAGE_COUNT] = {
    "a.bmp",
    "b.bmp",
    "c.bmp"};

/* 电子相册系统的状态结构体 */
typedef struct
{
    lv_obj_t *screen;    // 指向相册界面的根对象
    lv_obj_t *image;     // 指向展示照片的大图片控件
    lv_obj_t *tip_label; // 指向底部提示页码信息的文字标签
    int current_index;
    char pic_dir[PATH_MAX];
    char image_path[PATH_MAX]; // 缓存当前展示的完整图片路径
} album_ui_t;

// 定义静态全局变量，实例化相册系统状态
static album_ui_t g_album;

static lv_timer_t *auto_play_timer = NULL;
static lv_obj_t *auto_play_label = NULL; // 控制按钮上的文字变化

/* 通用按钮创建工厂函数 */
static lv_obj_t *create_text_button(lv_obj_t *parent, const char *text, lv_event_cb_t cb)
{
    lv_obj_t *button = lv_button_create(parent);                  // 创建按钮对象
    lv_obj_set_size(button, 96, 42);                              // 相册里的翻页按钮做小一点：96x42
    lv_obj_set_style_radius(button, 14, 0);                       // 圆角 14
    lv_obj_set_style_bg_color(button, lv_color_hex(0xAEEEEE), 0); // 蓝色背景
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);             // 不透明
    lv_obj_add_event_cb(button, cb, LV_EVENT_CLICKED, NULL);      // 绑定传入的点击回调函数

    lv_obj_t *label = lv_label_create(button); // 按钮内嵌文字
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_font(label, ui_font_get_22(), 0);
    lv_obj_center(label); // 文字绝对居中

    return button; // 返回按键控件的指针
}

/* 初始化图片目录（更新版：硬编码固定路径） */
static void init_pic_dir(void)
{
    if (g_album.pic_dir[0] != '\0')
    {
        return; // 防止被多次调用重复初始化
    }

    /* 相册页面直接使用固定图片目录
     * 把盘符 'A' 与 Ubuntu 虚拟机挂载的共享目录绝对路径焊死在一起
     */
    snprintf(
        g_album.pic_dir,
        sizeof(g_album.pic_dir),
        "A:/yu/2048_project/pic");
}

/* 根据 current_index 索引刷新中间的大图和底部的文字 */
static void refresh_album_image(void)
{
    char *dst = g_album.image_path;               // 指向需要被修改的路径字符串内存
    size_t dst_size = sizeof(g_album.image_path); // 字符串总容量
    size_t dir_len = strlen(g_album.pic_dir);     // 计算目录字符串长

    // 防御性编程：留出 16 字节的空间给文件名
    if (dir_len > dst_size - 16)
    {
        dir_len = dst_size - 16;
    }

    // 复制固定目录路径到目标字符串
    memcpy(dst, g_album.pic_dir, dir_len);
    dst[dir_len] = '\0'; // 加载字符串结束符

    // 追加文件名，例如拼接为：A:.../pic/b.bmp
    snprintf(dst + dir_len, dst_size - dir_len, "/%s", g_album_files[g_album.current_index]);

    // 通知 LVGL 的图片解码器，去这个新的路径读取并显示 BMP 图片
    lv_image_set_src(g_album.image, g_album.image_path);

    // 更新屏幕下方的文字进度，索引从 0 开始，所以展示给用户看的时候要 +1
    lv_label_set_text_fmt(
        g_album.tip_label,
        "第 %d / %d 张",
        g_album.current_index + 1,
        ALBUM_IMAGE_COUNT);
}

/* “上一张”按键被点击时的逻辑 */
static void on_prev_image(lv_event_t *e)
{
    LV_UNUSED(e);

    g_album.current_index--; // 当前数组下标 -1
    // 处理数组越界：如果小于0，就跳回最后一张图，实现向前循环播放
    if (g_album.current_index < 0)
    {
        g_album.current_index = ALBUM_IMAGE_COUNT - 1;
    }

    refresh_album_image(); // 计算完新索引后，调用刷新函数重新加载 UI
}

/* “下一张”按键被点击时的逻辑 */
static void on_next_image(lv_event_t *e)
{
    LV_UNUSED(e);

    g_album.current_index++; // 当前数组下标 +1
    // 处理数组越界：如果大等于总张数，就跳回第0张图，实现向后循环播放
    if (g_album.current_index >= ALBUM_IMAGE_COUNT)
    {
        g_album.current_index = 0;
    }

    refresh_album_image(); // 重新加载 UI
}

// 幻灯片定时器到了：直接强行调用“下一张”逻辑
static void on_auto_play_timer_cb(lv_timer_t *timer)
{
    // 因为 on_next_image 里面第一句是 LV_UNUSED(e)，所以传 NULL 是安全的
    on_next_image(NULL);
}

// 点击自动播放按钮的逻辑
static void on_auto_play_click(lv_event_t *e)
{
    LV_UNUSED(e);

    if (auto_play_timer == NULL)
    {
        // 开启轮播：创建一个 3000ms 触发一次的定时器
        auto_play_timer = lv_timer_create(on_auto_play_timer_cb, 3000, NULL);
        lv_label_set_text(auto_play_label, "停止播放"); // 改变按钮文字
    }
    else
    {
        // 停止轮播：删掉定时器
        lv_timer_del(auto_play_timer);
        auto_play_timer = NULL;
        lv_label_set_text(auto_play_label, "自动播放"); // 恢复按钮文字
    }
}

// 页面销毁回调：退出相册时必须干掉定时器，否则后台会野指针报错！
static void album_delete_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (auto_play_timer != NULL)
    {
        lv_timer_del(auto_play_timer);
        auto_play_timer = NULL;
    }
    g_album.screen = NULL;
}

/* 设置相册屏幕的通用背景风格 */
static void style_base_screen(lv_obj_t *screen)
{
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xF5F1EB), 0); // 相册使用浅米色作为底色
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);             // 完全不透明
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);            // 去除全局滚动条
}

/* 相册界面的主入口创建函数 */
lv_obj_t *album_create_screen(lv_event_cb_t back_cb)
{
    memset(&g_album, 0, sizeof(g_album)); // 初始化清空系统状态体
    init_pic_dir();                       // 装载固定图片路径

    g_album.screen = lv_obj_create(NULL); // 创建属于相册模块的独立屏幕
    style_base_screen(g_album.screen);    // 上底色

    // 给相册主屏幕绑定销毁事件，用来回收自动播放的定时器
    lv_obj_add_event_cb(g_album.screen, album_delete_cb, LV_EVENT_DELETE, NULL);

    /* 顶部大标题 */
    lv_obj_t *title = lv_label_create(g_album.screen);
    lv_label_set_text(title, "电子相册");
    lv_obj_set_style_text_color(title, lv_color_hex(0x5D5246), 0);
    lv_obj_set_style_text_font(title, ui_font_get_34(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18); // 靠上居中

    /* 右上角返回菜单按键，绑定外部传入的回调事件 */
    lv_obj_t *back_button = create_text_button(g_album.screen, "返回", back_cb);
    lv_obj_align(back_button, LV_ALIGN_TOP_RIGHT, -20, 16);

    /* 放置图片的主画框（边框容器） */
    lv_obj_t *frame = lv_obj_create(g_album.screen);
    lv_obj_set_size(frame, 620, 380);                            // 画框的固定尺寸设为 620x380
    lv_obj_align(frame, LV_ALIGN_CENTER, 0, 10);                 // 在屏幕中间稍微偏下排版
    lv_obj_set_style_radius(frame, 18, 0);                       // 画框加上 18 像素的圆角
    lv_obj_set_style_bg_color(frame, lv_color_hex(0xDDD4C7), 0); // 画框底色设为深米色
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(frame, 0, 0);
    lv_obj_set_style_pad_all(frame, 12, 0); // 画框留出 12 像素内边距，让内部的图片不贴边
    lv_obj_clear_flag(frame, LV_OBJ_FLAG_SCROLLABLE);

    /* 在画框内部创建展示用的图片控件 */
    g_album.image = lv_image_create(frame);
    lv_obj_center(g_album.image); // 图片控件在画框里面绝对居中

    /* 画框左边的“上一张”按钮 */
    lv_obj_t *prev_button = create_text_button(g_album.screen, "上一张", on_prev_image);
    // 对齐到画框的外部左侧居中，X 轴向左拉开 24 个像素
    lv_obj_align_to(prev_button, frame, LV_ALIGN_OUT_LEFT_MID, -24, 0);

    /* 画框右边的“下一张”按钮 */
    lv_obj_t *next_button = create_text_button(g_album.screen, "下一张", on_next_image);
    // 对齐到画框的外部右侧居中，X 轴向右拉开 24 个像素
    lv_obj_align_to(next_button, frame, LV_ALIGN_OUT_RIGHT_MID, 24, 0);

    /* 底部显示照片进度的文字标签 */
    g_album.tip_label = lv_label_create(g_album.screen);
    lv_obj_set_style_text_color(g_album.tip_label, lv_color_hex(0x7F7468), 0);
    lv_obj_set_style_text_font(g_album.tip_label, ui_font_get_22(), 0);
    // 依附在画框的正下方，向下偏移 18 像素
    lv_obj_align_to(g_album.tip_label, frame, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);

    // 自动播放
    lv_obj_t *auto_play_btn = lv_button_create(g_album.screen);
    lv_obj_set_size(auto_play_btn, 140, 46);
    lv_obj_set_style_radius(auto_play_btn, 14, 0);
    lv_obj_set_style_bg_color(auto_play_btn, lv_color_hex(0xAEEEEE), 0);
    // 把它钉在相册屏幕的正下方偏上一点点
    lv_obj_align(auto_play_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_add_event_cb(auto_play_btn, on_auto_play_click, LV_EVENT_CLICKED, NULL);

    auto_play_label = lv_label_create(auto_play_btn);
    lv_label_set_text(auto_play_label, "自动播放");
    lv_obj_set_style_text_color(auto_play_label, lv_color_black(), 0);
    lv_obj_set_style_text_font(auto_play_label, ui_font_get_22(), 0);
    lv_obj_center(auto_play_label);

    g_album.current_index = 0; // 程序一运行，默认显示下标为 0 的第一张图
    refresh_album_image();     // 初始化调用加载图片

    return g_album.screen; // 将搭建好的整页 UI 返回
}