#ifndef LOGGER_H
#define LOGGER_H

// ========================
// 日志记录模块
// ========================
//
// 将叫号、公告、连接、错误等事件写入 call_screen.log 文件。
// 日志格式：[2026-05-06 20:10:22] CALL number=012 name=张三 room=1诊室
//
// 所有写操作内部自动加锁 + fflush，多线程安全。
//
// 用法：
//   logger_write(LOG_CALL, "number=%d name=%s room=%s", 12, "张三", "1诊室");
//   logger_write(LOG_CONNECT, "客户端已连接 IP=%s", "192.168.1.5");

// 日志级别
typedef enum {
    LOG_CALL,       // 叫号事件
    LOG_NOTICE,     // 公告（弹出 / 关闭）
    LOG_CONNECT,    // 客户端连接 / 断开
    LOG_ERROR,      // 错误（JSON 解析失败、未知命令等）
} log_level_t;

// 写入一条日志（printf 风格可变参数）
// level 会被转为文字标签，fmt 之后的内容为日志正文
void logger_write(log_level_t level, const char *fmt, ...);

// 设置日志文件路径（默认 "./call_screen.log"）
void logger_set_path(const char *path);

#endif
