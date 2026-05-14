// 引入播放器头文件
#include "player.h"
// 引入LVGL图形库头文件
#include "../lvgl/lvgl.h"
// 标准输入输出库
#include <stdio.h>
// 字符串处理库
#include <string.h>
// 文件控制库，用于文件操作
#include <fcntl.h>
// Linux系统类型定义库
#include <sys/types.h>
// Linux文件状态库
#include <sys/stat.h>
// 标准库
#include <stdlib.h>
// 错误号定义库
#include <errno.h>
// Unix标准库，提供close、read等函数
#include <unistd.h>
// 进程等待库
#include <sys/wait.h>
// 信号处理库
#include <signal.h>

// 定义命名管道路径，用于和mplayer通信
#define FIFOPATH "/home/china/1.fifo"
// 视频播放列表
char *video_list[] = {"1.mp4", "2.mp4", "3.mp4"};
// 计算视频总数量
#define VIDEO_COUNT (sizeof(video_list)/sizeof(video_list[0]))

// 当前播放的视频索引
int current_index = 0;
// 命名管道文件描述符
int fdfifo = -1;
// 管道数组，用于读取mplayer输出
int pipe_fd[2];
// mplayer进程ID
pid_t mplayer_pid = -1;
// 暂停标志：0-未暂停，1-已暂停
int is_pause = 0;
// 音量值，默认100
int volume = 100;

// 视频总时长（秒）
float total_time = 0.0;
// 当前播放时长（秒）
float current_time = 0.0;

// LVGL控件对象：视频标题标签
lv_obj_t *label_title;
// LVGL控件对象：时间显示标签
lv_obj_t *label_time;
// LVGL控件对象：播放进度滑动条
lv_obj_t *slider_progress;
// LVGL控件对象：各种功能按钮
lv_obj_t *btn_play, *btn_prev, *btn_next, *btn_ff, *btn_rew, *btn_pause, *btn_vol_up, *btn_vol_down, *btn_exit;

// 定时器句柄：每秒刷新播放时间
static lv_timer_t *time_timer = NULL;

// 进度滑动条事件回调函数：拖动跳转播放
static void slider_progress_cb(lv_event_t *e)
{
    // 获取事件类型
    lv_event_code_t code = lv_event_get_code(e);
    // 只处理滑动条释放事件，避免频繁跳转
    if(code != LV_EVENT_RELEASED) return;

    // 管道未打开或视频时长无效，直接返回
    if(fdfifo < 0 || total_time <= 0) return;

    // 获取滑动条当前值（0~100）
    int val = lv_slider_get_value(slider_progress);
    // 根据百分比计算目标跳转时间
    float seek_time = (val / 100.0f) * total_time;

    // 定义命令缓冲区
    char cmd[64];
    // 拼接mplayer绝对跳转指令
    sprintf(cmd, "seek %.2f 2\n", seek_time);
    // 通过管道发送指令给mplayer
    write(fdfifo, cmd, strlen(cmd));

    // 同步更新当前播放时间
    current_time = seek_time;
    // 定义时间字符串缓冲区
    char time_str[64];
    // 格式化时间字符串
    sprintf(time_str, "%.2f / %.2f", current_time, total_time);
    // 更新界面时间显示
    lv_label_set_text(label_time, time_str);
}

// 停止当前播放的视频
void stop_current_video()
{
    // 如果mplayer进程存在
    if(mplayer_pid > 0)
    {
        // 发送退出指令给mplayer
        write(fdfifo, "quit\n", strlen("quit\n"));
        // 等待进程结束，避免僵尸进程
        waitpid(mplayer_pid, NULL, 0);
        // 重置进程ID
        mplayer_pid = -1;
    }
    // 如果管道已打开
    if(fdfifo >= 0)
    {
        // 关闭管道
        close(fdfifo);
        // 重置文件描述符
        fdfifo = -1;
    }

    // 如果定时器存在
    if(time_timer != NULL){
        // 删除定时器
        lv_timer_del(time_timer);
        // 重置定时器句柄
        time_timer = NULL;
    }

    // 如果进度条存在
    if(slider_progress != NULL){
        // 重置进度条为0
        lv_slider_set_value(slider_progress, 0, LV_ANIM_OFF);
    }
}

// 获取视频总时长
void get_video_total_time()
{
    // 管道未打开直接返回
    if(fdfifo < 0) return;

    // 初始化总时长为0
    total_time = 0.0;
    // 定义数据接收缓冲区
    char buff[128] = {0};

    // 发送查询总时长指令给mplayer
    write(fdfifo, "get_time_length\n", strlen("get_time_length\n"));

    // 循环读取管道数据
    while(1)
    {
        // 非阻塞读取管道数据
        int ret = read(pipe_fd[0], buff, sizeof(buff)-1);
        // 读取到有效数据且包含总时长信息
        if(ret > 0 && strstr(buff, "ANS_LENGTH="))
        {
            // 解析总时长
            sscanf(buff, "ANS_LENGTH=%f", &total_time);
            // 退出循环
            break;
        }
    }
    // 打印总时长信息
    printf("总时长：%s\n",buff);
}

// 定时器回调函数：每秒刷新当前播放时间和进度条
static void time_timer_cb(lv_timer_t *timer)
{
    // 管道、进程或时长无效，直接返回
    if(fdfifo < 0 || mplayer_pid < 0 || total_time <= 0) return;

    // 定义数据接收缓冲区
    char buff[128] = {0};
    // 初始化当前时间为0
    current_time = 0.0;

    // 发送查询当前播放时间指令
    write(fdfifo, "get_time_pos\n", strlen("get_time_pos\n"));

    // 循环读取管道数据
    while(1)
    {
        // 非阻塞读取管道数据
        int ret = read(pipe_fd[0], buff, sizeof(buff)-1);
        // 读取到有效数据且包含当前时间信息
        if(ret > 0 && strstr(buff, "ANS_TIME_POSITION="))
        {
            // 解析当前播放时间
            sscanf(buff, "ANS_TIME_POSITION=%f", &current_time);
            // 退出循环
            break;
        }
    }

    // 定义时间字符串缓冲区
    char time_str[64];
    // 格式化时间显示字符串
    sprintf(time_str, "%.2f / %.2f", current_time, total_time);
    // 更新界面时间标签
    lv_label_set_text(label_time, time_str);

    // 计算播放进度百分比
    int progress = (current_time / total_time) * 100;
    // 进度值合法时更新进度条
    if(progress >= 0 && progress <= 100){
        lv_slider_set_value(slider_progress, progress, LV_ANIM_OFF);
    }
}

// 播放当前选中的视频
void start_play_current()
{
    // 先停止当前播放
    stop_current_video();

    // 检查管道文件是否存在，不存在则创建
    if(access(FIFOPATH, F_OK) == -1)
        mkfifo(FIFOPATH, 0666);

    // 以读写方式打开命名管道
    fdfifo = open(FIFOPATH, O_RDWR);
    // 打开失败打印错误并返回
    if(fdfifo < 0)
    {
        perror("open fifo");
        return;
    }

    // 创建匿名管道，用于读取mplayer输出
    if(pipe(pipe_fd) < 0)
    {
        perror("pipe");
        return;
    }

    // 获取管道读端标志
    int flags = fcntl(pipe_fd[0], F_GETFL, 0);
    // 设置管道读端为非阻塞模式
    fcntl(pipe_fd[0], F_SETFL, flags | O_NONBLOCK);

    // 创建子进程运行mplayer
    mplayer_pid = fork();
    // 子进程
    if(mplayer_pid == 0)
    {
        // 将标准输出重定向到管道写端
        dup2(pipe_fd[1], STDOUT_FILENO);
        // 关闭管道读端
        close(pipe_fd[0]);

        // 定义管道命令缓冲区
        char fifo_cmd[128];
        // 拼接mplayer输入管道命令
        sprintf(fifo_cmd, "file=%s", FIFOPATH);

        // 执行mplayer播放器进程
        // 参数：-slave从机模式 -quiet安静模式 -input指定控制管道
        // -zoom自适应分辨率 -x/-y指定窗口大小 -geometry指定窗口位置
        execlp("mplayer", "mplayer", "-slave", "-quiet", "-input", fifo_cmd,
               "-zoom", "-x", "1024", "-y", "480", "-geometry", "0:0",
               video_list[current_index], NULL);
        // 子进程退出
        exit(0);
    }
    // 父进程
    else if(mplayer_pid > 0)
    {
        // 关闭管道写端
        close(pipe_fd[1]);
        // 更新界面视频标题
        lv_label_set_text(label_title, video_list[current_index]);

        // 获取视频总时长
        get_video_total_time();

        // 重置暂停状态
        is_pause = 0;

        // 创建定时器，1秒执行一次刷新函数
        time_timer = lv_timer_create(time_timer_cb, 1000, NULL);
    }
}

//==================== 按钮回调函数 ====================
// 播放按钮回调
static void btn_play_cb(lv_event_t * e)
{
    // 开始播放当前视频
    start_play_current();
}

// 上一个视频按钮回调
static void btn_prev_cb(lv_event_t * e)
{
    // 索引减1
    current_index--;
    // 索引越界则循环到最后一个
    if(current_index < 0) current_index = VIDEO_COUNT - 1;
    // 更新标题
    lv_label_set_text(label_title, video_list[current_index]);
    // 播放新视频
    start_play_current();
}

// 下一个视频按钮回调
static void btn_next_cb(lv_event_t * e)
{
    // 索引加1
    current_index++;
    // 索引越界则循环到第一个
    if(current_index >= VIDEO_COUNT) current_index = 0;
    // 更新标题
    lv_label_set_text(label_title, video_list[current_index]);
    // 播放新视频
    start_play_current();
}

// 快进5秒按钮回调
static void btn_ff_cb(lv_event_t * e)
{
    // 管道有效则发送快进指令
    if(fdfifo >= 0) write(fdfifo, "seek +5\n", 8);
}

// 快退5秒按钮回调
static void btn_rew_cb(lv_event_t * e)
{
    // 管道有效则发送快退指令
    if(fdfifo >= 0) write(fdfifo, "seek -5\n", 8);
}

// 暂停/播放按钮回调
static void btn_pause_cb(lv_event_t * e)
{
    // 管道有效
    if(fdfifo >= 0)
    {
        // 发送暂停/切换指令
        write(fdfifo, "pause\n", 6);
        // 切换暂停状态
        is_pause = !is_pause;
        // 暂停时停止定时器，播放时恢复定时器
        if(is_pause)
            lv_timer_pause(time_timer);
        else
            lv_timer_resume(time_timer);
    }
}

// 音量加按钮回调
static void btn_vol_up_cb(lv_event_t * e)
{
    // 管道有效
    if(fdfifo >= 0)
    {
        // 音量小于100时增加10
        if(volume<100)
            volume+=10;
        // 定义音量指令缓冲区
        char cmd[100] = {0};
        // 拼接音量设置指令
        sprintf(cmd,"volume %d 1\n",volume);
        // 发送指令
        write(fdfifo, cmd, strlen(cmd));
    }
}

// 音量减按钮回调
static void btn_vol_down_cb(lv_event_t * e)
{
    // 管道有效
    if(fdfifo >= 0)
    {
        // 音量大于0时减少10
        if(volume>0)
            volume-=10;
        // 定义音量指令缓冲区
        char cmd[100] = {0};
        // 拼接音量设置指令
        sprintf(cmd,"volume %d 1\n",volume);
        // 发送指令
        write(fdfifo, cmd, strlen(cmd));
    }
}

// 退出按钮回调
static void btn_exit_cb(lv_event_t * e)
{
    // 停止视频播放
    stop_current_video();
    // 删除命名管道文件
    unlink(FIFOPATH);
}

//==================== 创建播放器UI界面 ====================
void create_video_player_ui(void)
{
    // 创建视频标题标签，挂载到当前屏幕
    label_title = lv_label_create(lv_scr_act());
    // 设置默认文本
    lv_label_set_text(label_title, "No Video");
    // 对齐到顶部中间
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 10);
    // 设置字体大小
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_20, 0);

    // 创建时间显示标签
    label_time = lv_label_create(lv_scr_act());
    // 设置默认时间文本
    lv_label_set_text(label_time, "0.00 / 0.00");
    // 对齐到顶部中间，标题下方
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 40);
    // 设置字体大小
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_20, 0);

    // ===================== 创建播放进度滑动条 =====================
    slider_progress = lv_slider_create(lv_scr_act());
    // 设置宽度
    lv_obj_set_width(slider_progress, 980);
    // 对齐到底部中间
    lv_obj_align(slider_progress, LV_ALIGN_BOTTOM_MID, 0, -70);
    // 设置进度条范围0~100
    lv_slider_set_range(slider_progress, 0, 100);
    // 设置初始值为0
    lv_slider_set_value(slider_progress, 0, LV_ANIM_OFF);
    // 为进度条添加事件回调
    lv_obj_add_event_cb(slider_progress, slider_progress_cb, LV_EVENT_ALL, NULL);

    // ===================== 创建底部控制按钮 =====================
    // 按钮宽度
    int btn_w = 75;
    // 按钮高度
    int btn_h = 40;
    // 按钮Y轴位置
    int y_pos = -20;

    // 1. 创建上一个按钮
    btn_prev = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_prev, btn_w, btn_h);
    lv_obj_align(btn_prev, LV_ALIGN_BOTTOM_MID, -400, y_pos);
    lv_obj_add_event_cb(btn_prev, btn_prev_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_prev); lv_label_set_text(lv_obj_get_child(btn_prev, 0), "Prev");

    // 2. 创建播放按钮
    btn_play = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_play, btn_w, btn_h);
    lv_obj_align(btn_play, LV_ALIGN_BOTTOM_MID, -300, y_pos);
    lv_obj_add_event_cb(btn_play, btn_play_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_play); lv_label_set_text(lv_obj_get_child(btn_play, 0), "Play");

    // 3. 创建暂停按钮
    btn_pause = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_pause, btn_w, btn_h);
    lv_obj_align(btn_pause, LV_ALIGN_BOTTOM_MID, -200, y_pos);
    lv_obj_add_event_cb(btn_pause, btn_pause_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_pause); lv_label_set_text(lv_obj_get_child(btn_pause, 0), "Pause");

    // 4. 创建下一个按钮
    btn_next = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_next, btn_w, btn_h);
    lv_obj_align(btn_next, LV_ALIGN_BOTTOM_MID, -100, y_pos);
    lv_obj_add_event_cb(btn_next, btn_next_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_next); lv_label_set_text(lv_obj_get_child(btn_next, 0), "Next");

    // 5. 创建快退按钮
    btn_rew = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_rew, btn_w, btn_h);
    lv_obj_align(btn_rew, LV_ALIGN_BOTTOM_MID, 0, y_pos);
    lv_obj_add_event_cb(btn_rew, btn_rew_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_rew); lv_label_set_text(lv_obj_get_child(btn_rew, 0), "Rew");

    // 6. 创建快进按钮
    btn_ff = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_ff, btn_w, btn_h);
    lv_obj_align(btn_ff, LV_ALIGN_BOTTOM_MID, 100, y_pos);
    lv_obj_add_event_cb(btn_ff, btn_ff_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_ff); lv_label_set_text(lv_obj_get_child(btn_ff, 0), "FF");

    // 7. 创建音量加按钮
    btn_vol_up = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_vol_up, btn_w, btn_h);
    lv_obj_align(btn_vol_up, LV_ALIGN_BOTTOM_MID, 200, y_pos);
    lv_obj_add_event_cb(btn_vol_up, btn_vol_up_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_vol_up); lv_label_set_text(lv_obj_get_child(btn_vol_up, 0), "Vol+");

    // 8. 创建音量减按钮
    btn_vol_down = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_vol_down, btn_w, btn_h);
    lv_obj_align(btn_vol_down, LV_ALIGN_BOTTOM_MID, 300, y_pos);
    lv_obj_add_event_cb(btn_vol_down, btn_vol_down_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_vol_down); lv_label_set_text(lv_obj_get_child(btn_vol_down, 0), "Vol-");

    // 9. 创建退出按钮
    btn_exit = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn_exit, btn_w, btn_h);
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 400, y_pos);
    lv_obj_add_event_cb(btn_exit, btn_exit_cb, LV_EVENT_CLICKED, NULL);
    lv_label_create(btn_exit); lv_label_set_text(lv_obj_get_child(btn_exit, 0), "Exit");
}