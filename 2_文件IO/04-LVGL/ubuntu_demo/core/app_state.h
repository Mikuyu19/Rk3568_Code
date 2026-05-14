#ifndef APP_STATE_H
#define APP_STATE_H

#include <pthread.h>

/* 排队队列最大容量 */
#define QUEUE_MAX_COUNT 10

/*
 * 医生信息结构体
 *   room  - 所属诊室，如 "1诊室"
 *   name  - 医生姓名，如 "王医生"
 *   title - 职称，如 "主任医师"
 */
typedef struct {
    char room[16];
    char name[32];
    char title[32];
} doctor_info_t;

/*
 * 病人信息结构体
 *   number - 排队号码
 *   name   - 病人姓名
 *   room   - 分诊诊室
 */
typedef struct {
    int number;
    char name[32];
    char room[16];
} patient_info_t;

/*
 * ========================
 * 全局共享状态模块
 * ========================
 *
 * 本模块是所有数据的中转站，负责：
 *   1. 存储当前叫号、候诊队列、医生信息、紧急公告
 *   2. 用 pthread_mutex_t 保护所有数据读写
 *   3. 通过"标志位机制"解耦网络线程和 LVGL UI 线程
 *
 * 两条铁律（线程安全）：
 *   → 网络线程只调 Setter 写入数据（内部自动加锁）
 *   → LVGL 线程只调 Checker + Getter 读取数据（内部自动加锁）
 *   → 任何线程都不要直接访问本模块的 static 全局变量
 *
 * 数据流示例：
 *   护士站发来 @call 命令
 *     → net_server 线程解析命令
 *     → app_state_set_call() 写入数据 + 置 new_call_flag = 1
 *     → LVGL 定时器轮询 app_state_has_new_call() 发现标志
 *     → app_state_get_call() 取出数据
 *     → 调用 medical.c 的 update_current_call() 刷新画面
 */

/* ============ 初始化（程序启动时调一次） ============ */

/*  加载默认模拟数据（叫号、医生、排队），初始化互斥锁 */
void app_state_init(void);

/* ============ Setter：网络线程调用 ============ */

/*
 * 写入当前叫号信息
 *   number - 叫到的号码
 *   name   - 病人姓名
 *   room   - 诊室
 *   doctor - 医生姓名（可选，传 NULL 则不更新）
 *   title  - 医生职称（可选，传 NULL 则不更新）
 * 副作用：自动从排队队列中移除该号码的病人
 */
void app_state_set_call(int number, const char *name, const char *room,
                        const char *doctor, const char *title);

/*
 * 整批替换候诊队列
 *   queue - 病人数组指针
 *   count - 数组长度（不能超过 QUEUE_MAX_COUNT）
 */
void app_state_set_queue(const patient_info_t *queue, int count);

/*
 * 更新指定位置的医生信息
 *   index - 医生下标（0 或 1）
 *   name / title / room - 要更新的字段，传 NULL 表示不修改该字段
 */
void app_state_set_doctor(int index, const char *name, const char *title,
                          const char *room);

/*  设置紧急公告文本和显示时长（秒） */
void app_state_set_notice(const char *text, int duration_sec);

/*  通知 UI 关闭当前公告 */
void app_state_clear_notice(void);

/* ============ Checker：LVGL 定时器轮询 ============ */
/*
 * 每个 checker 返回 1 表示"有新数据需要刷新界面"
 * 调用后标志位自动清零（不会重复触发）
 */

int app_state_has_new_call(void);       /* 有新叫号？ */
int app_state_has_new_queue(void);      /* 候诊队列有变化？ */
int app_state_has_new_doctor(void);     /* 医生信息有变化？ */
int app_state_has_new_notice(void);     /* 有新公告要显示？ */
int app_state_has_clear_notice(void);   /* 需要关闭公告？ */

/* ============ Getter：LVGL 线程读取 ============ */

/*  取出当前叫号病人 + 对应诊室医生，out/doc_out 为输出参数 */
void app_state_get_call(patient_info_t *out, doctor_info_t *doc_out);

/*  返回当前排队人数 */
int  app_state_get_queue_count(void);

/*  取出排队队列中第 index 个病人（0 为队首） */
void app_state_get_queue_item(int index, patient_info_t *out);

/*  返回医生总数 */
int  app_state_get_doctor_count(void);

/*  取出第 index 个医生的信息 */
void app_state_get_doctor(int index, doctor_info_t *out);

/*  取出公告文本和显示时长 */
void app_state_get_notice(char *text_out, int max_len, int *duration_out);

/* ============ 本地操作：LVGL 线程直接调用 ============ */
/*
 * 以下函数由触屏 UI 事件触发（点列表叫号、填表单添加病人）
 * 这些操作发生在 LVGL 主线程中，和网络线程通过互斥锁隔离
 */

/*  叫号：弹出排队队列中第 index 个病人，失败返回 -1 */
int  app_state_call_by_index(int index);

/*  添加病人到排队队列，urgent=1 表示急诊插队到最前面 */
void app_state_add_patient(const char *name, const char *room, int urgent);

/*  获取下一个可分配的号码 */
int  app_state_get_next_number(void);

#endif
