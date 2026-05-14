// ========================
// logger.c —— 日志记录实现
// ========================
//
// 每调一次 logger_write() 就在文件末尾追加一行。
// 日志格式：[2026-05-06 20:10:22] CALL number=012 name=张三 room=1诊室
//
// 线程安全：内部分别对文件打开/写入/关闭加锁，
//          确保多线程同时写不会交错。
//
// 防丢数据：每次写入后立刻 fflush()，
//          即使程序意外崩溃也能保留已写入的日志。

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

// ======================== 常量 ========================

#define DEFAULT_PATH "./call_screen.log"
#define TIME_BUF_SIZE   64      // 时间字符串缓冲区

// ======================== 全局变量 ========================

static char g_path[256] = DEFAULT_PATH;  // 日志文件路径
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

// ======================== 工具函数 ========================

// 将 log_level_t 转为日志中的文字标签
static const char *level_to_str(log_level_t level)
{
    switch (level) {
        case LOG_CALL:    return "CALL";
        case LOG_NOTICE:  return "NOTICE";
        case LOG_CONNECT: return "CONNECT";
        case LOG_ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

// 生成当前时间字符串，格式 [YYYY-MM-DD HH:MM:SS]
static void make_timestamp(char *buf, int buf_size)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buf, buf_size, "[%Y-%m-%d %H:%M:%S]", tm_info);
}

// ======================== 公开接口 ========================

void logger_write(log_level_t level, const char *fmt, ...)
{
    char time_str[TIME_BUF_SIZE];
    char msg[512];  // 格式化后的日志正文

    // 1. 在锁外生成时间戳和正文（这些不涉及共享资源）
    make_timestamp(time_str, sizeof(time_str));

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    // 2. 加锁 → 打开文件 → 写入 → fflush → 关闭 → 解锁
    pthread_mutex_lock(&g_mutex);

    FILE *fp = fopen(g_path, "a");  // 追加模式
    if (fp == NULL) {
        perror("[logger] fopen 失败");  // 打印错误原因
        fprintf(stderr, "[logger] 日志路径: %s\n", g_path);
        pthread_mutex_unlock(&g_mutex);
        return;
    }

    fprintf(fp, "%s %s %s\n", time_str, level_to_str(level), msg);
    fflush(fp);  // 立即刷盘，不怕崩溃丢数据
    fclose(fp);

    pthread_mutex_unlock(&g_mutex);
}

void logger_set_path(const char *path)
{
    if (path == NULL || path[0] == '\0') {
        return;
    }

    pthread_mutex_lock(&g_mutex);
    snprintf(g_path, sizeof(g_path), "%s", path);
    pthread_mutex_unlock(&g_mutex);

    printf("[logger] 日志路径: %s\n", g_path);
}
