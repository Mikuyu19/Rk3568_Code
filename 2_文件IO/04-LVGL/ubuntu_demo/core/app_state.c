/*
 * app_state.c —— 全局共享状态模块
 *
 * 本文件管理医疗叫号屏的所有运行时数据，是网络层和 UI 层之间的唯一数据通道。
 *
 * 架构：
 *   ┌──────────┐     ┌───────────────┐     ┌──────────┐
 *   │ 网络线程  │ ──→ │  app_state    │ ──→ │ LVGL 线程│
 *   │ (setter) │     │ (static 变量)  │     │ (getter) │
 *   └──────────┘     └───────────────┘     └──────────┘
 *                           │
 *                     pthread_mutex_t
 *                     (互斥锁保护)
 *
 * 所有 public 函数开头以 app_state_ 为前缀，内部自动加锁/解锁。
 */

#include "app_state.h"
#include <string.h>
#include <stdio.h>

/* ==================================================================
 * 全局状态变量（全部 static，外部 .c 文件不可直接访问）
 * ================================================================== */

static patient_info_t g_current_patient;          /* 当前正在叫号的病人 */
static patient_info_t g_wait_queue[QUEUE_MAX_COUNT]; /* 候诊排队数组 */
static doctor_info_t  g_doctors[2];               /* 医生数组（2 个诊室） */
static int            g_doctor_count = 2;          /* 医生总数 */

static char  g_notice_text[256];   /* 紧急公告文本内容 */
static int   g_notice_duration;    /* 公告显示时长（秒），0 表示手动关闭 */

static int g_queue_count = 0;      /* 当前排队人数 */
static int g_next_number = 1;      /* 下一个要分配的排队号码（自动递增） */

/*
 * 事件标志位 —— 解耦网络线程和 UI 线程的关键
 *
 * 网络线程 setter 写数据后置为 1
 * LVGL 定时器 checker 检测到 1 后消费（读取并清零）
 *
 * 这样 LVGL 不需要阻塞等待网络数据，
 * 网络线程也不需要在数据到来时直接操作 LVGL 控件（那会线程不安全）。
 */
static int g_new_call_flag    = 0;  /* 有新叫号需要刷新 */
static int g_new_queue_flag   = 0;  /* 候诊队列有变化 */
static int g_new_doctor_flag  = 0;  /* 医生信息有更新 */
static int g_new_notice_flag  = 0;  /* 需要弹出公告 */
static int g_clear_notice_flag = 0; /* 需要关闭公告 */

/* 互斥锁 —— 保护上面所有全局变量 */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ==================================================================
 * 初始化
 * ================================================================== */

void app_state_init(void)
{
    pthread_mutex_lock(&g_mutex);

    /* ----- 设置默认当前叫号 ----- */
    g_current_patient.number = 12;
    snprintf(g_current_patient.name, sizeof(g_current_patient.name),
             "%s", "蔡徐坤");
    snprintf(g_current_patient.room, sizeof(g_current_patient.room),
             "%s", "1诊室");

    /* ----- 设置默认医生信息（两个诊室各一位） ----- */
    snprintf(g_doctors[0].room,  sizeof(g_doctors[0].room),  "%s", "1诊室");
    snprintf(g_doctors[0].name,  sizeof(g_doctors[0].name),  "%s", "王医生");
    snprintf(g_doctors[0].title, sizeof(g_doctors[0].title), "%s", "主任医师");

    snprintf(g_doctors[1].room,  sizeof(g_doctors[1].room),  "%s", "2诊室");
    snprintf(g_doctors[1].name,  sizeof(g_doctors[1].name),  "%s", "李医生");
    snprintf(g_doctors[1].title, sizeof(g_doctors[1].title), "%s", "副主任医师");

    /* ----- 填充默认候诊队列（6 个排队的病人） ----- */
    patient_info_t defaults[] = {
        {13, "丁真",   "1诊室"},
        {14, "马嘉祺", "1诊室"},
        {15, "刘耀文", "2诊室"},
        {16, "张真源", "1诊室"},
        {17, "宋亚轩", "2诊室"},
        {18, "严浩翔", "1诊室"},
    };
    g_queue_count = 6;
    g_next_number = 19;  /* 下一个新加入的病人从 19 号开始 */
    for (int i = 0; i < g_queue_count; i++) {
        g_wait_queue[i] = defaults[i];
    }

    /* ----- 所有标志位归零 ----- */
    g_new_call_flag     = 0;
    g_new_queue_flag    = 0;
    g_new_doctor_flag   = 0;
    g_new_notice_flag   = 0;
    g_clear_notice_flag = 0;

    /* 公告初始为空 */
    g_notice_text[0] = '\0';
    g_notice_duration = 0;

    pthread_mutex_unlock(&g_mutex);
}

/* ==================================================================
 * Setter —— 网络线程调用
 *
 * 每个 setter 的三步曲：
 *   1. 加锁
 *   2. 修改数据 + 置对应标志位
 *   3. 解锁
 * ================================================================== */

/*
 * app_state_set_call —— 护士站远程叫号
 *
 * 收到远程叫号命令后调用此函数，会同时：
 *   - 更新当前叫号大屏显示
 *   - 更新对应诊室的医生姓名和职称
 *   - 如果该病人在排队列表中，自动移除（已经叫到了）
 */
void app_state_set_call(int number, const char *name, const char *room,
                        const char *doctor, const char *title)
{
    pthread_mutex_lock(&g_mutex);

    /* 1. 写入当前叫号病人 */
    g_current_patient.number = number;
    snprintf(g_current_patient.name, sizeof(g_current_patient.name),
             "%s", name ? name : "");
    snprintf(g_current_patient.room, sizeof(g_current_patient.room),
             "%s", room ? room : "");

    /* 2. 同步更新对应诊室的医生信息 */
    int idx = 0;
    if (room && strstr(room, "2")) {
        idx = 1;  /* 2 诊室对应 doctors[1] */
    }
    if (doctor && doctor[0]) {
        snprintf(g_doctors[idx].name, sizeof(g_doctors[idx].name),
                 "%s", doctor);
    }
    if (title && title[0]) {
        snprintf(g_doctors[idx].title, sizeof(g_doctors[idx].title),
                 "%s", title);
    }

    /* 3. 如果该号码在排队列表中，把它移除（人已经叫到了就不用排了） */
    for (int i = 0; i < g_queue_count; i++) {
        if (g_wait_queue[i].number == number) {
            /* 后面的病人全部往前挪一位，覆盖当前位置 */
            for (int j = i; j < g_queue_count - 1; j++) {
                g_wait_queue[j] = g_wait_queue[j + 1];
            }
            g_queue_count--;
            break;  /* 号码唯一，找到一个就可以停了 */
        }
    }

    /* 4. 升起"新叫号"旗帜，等待 LVGL 定时器来取 */
    g_new_call_flag = 1;
    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_set_queue —— 护士站远程更新整条候诊队列
 *
 * 适用场景：护士站推送最新的完整排队列表
 */
void app_state_set_queue(const patient_info_t *queue, int count)
{
    pthread_mutex_lock(&g_mutex);

    /* 防止越界 */
    g_queue_count = (count > QUEUE_MAX_COUNT) ? QUEUE_MAX_COUNT : count;
    for (int i = 0; i < g_queue_count; i++) {
        g_wait_queue[i] = queue[i];
    }

    g_new_queue_flag = 1;
    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_set_doctor —— 更新医生信息
 *
 * 三个参数都是可选的：传 NULL 或空字符串表示"这个字段不改"
 */
void app_state_set_doctor(int index, const char *name, const char *title,
                          const char *room)
{
    pthread_mutex_lock(&g_mutex);

    if (index >= 0 && index < g_doctor_count) {
        if (name  && name[0])  snprintf(g_doctors[index].name,
                                        sizeof(g_doctors[index].name),
                                        "%s", name);
        if (title && title[0]) snprintf(g_doctors[index].title,
                                        sizeof(g_doctors[index].title),
                                        "%s", title);
        if (room  && room[0])  snprintf(g_doctors[index].room,
                                        sizeof(g_doctors[index].room),
                                        "%s", room);
    }

    g_new_doctor_flag = 1;
    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_set_notice —— 推送紧急公告
 *
 * duration_sec: 显示持续秒数，0 表示需要手动关闭
 */
void app_state_set_notice(const char *text, int duration_sec)
{
    pthread_mutex_lock(&g_mutex);

    snprintf(g_notice_text, sizeof(g_notice_text), "%s", text ? text : "");
    g_notice_duration = duration_sec;
    g_new_notice_flag = 1;  /* 通知 UI："有公告来了，弹出来！" */

    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_clear_notice —— 远程关闭公告
 */
void app_state_clear_notice(void)
{
    pthread_mutex_lock(&g_mutex);

    g_clear_notice_flag = 1;  /* 通知 UI："把公告关掉" */

    pthread_mutex_unlock(&g_mutex);
}

/* ==================================================================
 * Checker —— LVGL 定时器轮询
 *
 * 每个 checker 做的事：
 *   1. 加锁
 *   2. 读取标志位 → 保存到局部变量
 *   3. 立刻清零标志位（防止下次轮询重复处理）
 *   4. 解锁
 *   5. 返回保存的标志位值
 *
 * 调用方拿到 1 就去调用对应的 getter 取数据并刷新界面。
 * ================================================================== */

int app_state_has_new_call(void)
{
    pthread_mutex_lock(&g_mutex);
    int flag = g_new_call_flag;
    g_new_call_flag = 0;   /* 消费标志：只通知一次 */
    pthread_mutex_unlock(&g_mutex);
    return flag;
}

int app_state_has_new_queue(void)
{
    pthread_mutex_lock(&g_mutex);
    int flag = g_new_queue_flag;
    g_new_queue_flag = 0;
    pthread_mutex_unlock(&g_mutex);
    return flag;
}

int app_state_has_new_doctor(void)
{
    pthread_mutex_lock(&g_mutex);
    int flag = g_new_doctor_flag;
    g_new_doctor_flag = 0;
    pthread_mutex_unlock(&g_mutex);
    return flag;
}

int app_state_has_new_notice(void)
{
    pthread_mutex_lock(&g_mutex);
    int flag = g_new_notice_flag;
    g_new_notice_flag = 0;
    pthread_mutex_unlock(&g_mutex);
    return flag;
}

int app_state_has_clear_notice(void)
{
    pthread_mutex_lock(&g_mutex);
    int flag = g_clear_notice_flag;
    g_clear_notice_flag = 0;
    pthread_mutex_unlock(&g_mutex);
    return flag;
}

/* ==================================================================
 * Getter —— LVGL 线程读取
 *
 * 读取共享数据并拷贝到调用方提供的输出缓冲区。
 * 拷贝在锁内完成，保证读到的是完整、一致的数据。
 * ================================================================== */

/*
 * app_state_get_call —— 取当前叫号 + 对应医生
 *
 * out     - 输出参数，接收当前叫号病人信息（可为 NULL）
 * doc_out - 输出参数，接收该诊室的医生信息（可为 NULL）
 */
void app_state_get_call(patient_info_t *out, doctor_info_t *doc_out)
{
    pthread_mutex_lock(&g_mutex);

    if (out) {
        *out = g_current_patient;  /* 结构体直接赋值拷贝 */
    }

    if (doc_out) {
        /* 根据当前病人的诊室号匹配对应医生 */
        int idx = 0;
        if (strstr(g_current_patient.room, "2")) {
            idx = 1;
        }
        *doc_out = g_doctors[idx];
    }

    pthread_mutex_unlock(&g_mutex);
}

int app_state_get_queue_count(void)
{
    pthread_mutex_lock(&g_mutex);
    int count = g_queue_count;
    pthread_mutex_unlock(&g_mutex);
    return count;
}

/*
 * app_state_get_queue_item —— 取排队队列第 index 项
 *
 * index: 0 是队首（下一个要被叫到的人）
 */
void app_state_get_queue_item(int index, patient_info_t *out)
{
    pthread_mutex_lock(&g_mutex);

    if (out && index >= 0 && index < g_queue_count) {
        *out = g_wait_queue[index];
    }

    pthread_mutex_unlock(&g_mutex);
}

int app_state_get_doctor_count(void)
{
    /* 医生数量目前是常量 2，直接返回无需加锁 */
    return g_doctor_count;
}

void app_state_get_doctor(int index, doctor_info_t *out)
{
    pthread_mutex_lock(&g_mutex);

    if (out && index >= 0 && index < g_doctor_count) {
        *out = g_doctors[index];
    }

    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_get_notice —— 读取公告内容和时长
 *
 * text_out     - 输出缓冲区，接收公告文本
 * max_len      - 缓冲区大小
 * duration_out - 输出参数，接收显示时长（秒）
 */
void app_state_get_notice(char *text_out, int max_len, int *duration_out)
{
    pthread_mutex_lock(&g_mutex);

    if (text_out && max_len > 0) {
        snprintf(text_out, max_len, "%s", g_notice_text);
    }
    if (duration_out) {
        *duration_out = g_notice_duration;
    }

    pthread_mutex_unlock(&g_mutex);
}

/* ==================================================================
 * 本地操作 —— LVGL 线程直接调用
 *
 * 这些函数在触屏点击事件中调用（"模拟叫号"按钮、"添加病人"表单等）。
 * 因为调用方本身就在 LVGL 主线程里，不存在和网络线程的竞争，
 * 但为了保证数据一致性，内部依然加锁。
 * ================================================================== */

/*
 * app_state_call_by_index —— 触屏点选叫号
 *
 * 点列表第 index 行 → 该病人成为"当前叫号" → 从队列中移除
 * 返回 0 成功，-1 失败（index 越界或队列为空）
 */
int app_state_call_by_index(int index)
{
    pthread_mutex_lock(&g_mutex);

    if (g_queue_count <= 0 || index < 0 || index >= g_queue_count) {
        pthread_mutex_unlock(&g_mutex);
        return -1;
    }

    /* 把第 index 个病人提出来作为当前叫号 */
    g_current_patient = g_wait_queue[index];

    /* 后面的病人依次前移，填补空位 */
    for (int i = index; i < g_queue_count - 1; i++) {
        g_wait_queue[i] = g_wait_queue[i + 1];
    }
    g_queue_count--;

    /* 根据新病人的诊室号自动匹配医生 */
    int doc_idx = 0;
    if (strstr(g_current_patient.room, "2")) {
        doc_idx = 1;
    }

    g_new_call_flag = 1;  /* 通知 UI 定时器刷新 */

    pthread_mutex_unlock(&g_mutex);
    return 0;
}

/*
 * app_state_add_patient —— 触屏添加病人
 *
 * urgent = 0 → 加在队尾
 * urgent = 1 → 插队到队首（急诊病人）
 */
void app_state_add_patient(const char *name, const char *room, int urgent)
{
    pthread_mutex_lock(&g_mutex);

    if (g_queue_count >= QUEUE_MAX_COUNT) {
        pthread_mutex_unlock(&g_mutex);
        return;  /* 队列满了，加不了 */
    }

    int insert_idx = g_queue_count;  /* 默认插在队尾 */

    if (urgent && g_queue_count > 0) {
        /* 急诊插队：所有现有病人往后挪一位，把队首位置空出来 */
        insert_idx = 0;
        for (int i = g_queue_count; i > 0; i--) {
            g_wait_queue[i] = g_wait_queue[i - 1];
        }
    }

    /* 填入新病人的信息 */
    snprintf(g_wait_queue[insert_idx].name,
             sizeof(g_wait_queue[insert_idx].name), "%s", name);
    snprintf(g_wait_queue[insert_idx].room,
             sizeof(g_wait_queue[insert_idx].room), "%s", room);
    g_wait_queue[insert_idx].number = g_next_number++;  /* 分配号码，自增备用 */

    g_queue_count++;
    g_new_queue_flag = 1;  /* 通知 UI 定时器刷新队列列表 */

    pthread_mutex_unlock(&g_mutex);
}

/*
 * app_state_get_next_number —— 查询下一个可分配号码
 *
 * 供 medical.c 在添加病人表单中预显示号码
 */
int app_state_get_next_number(void)
{
    pthread_mutex_lock(&g_mutex);
    int n = g_next_number;
    pthread_mutex_unlock(&g_mutex);
    return n;
}
