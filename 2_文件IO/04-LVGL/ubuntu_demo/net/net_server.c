// ========================
// net_server.c —— TCP 服务端线程
// ========================
//
// 本模块负责：
//   socket() → bind() → listen() → epoll 循环
//   接收护士站客户端发来的命令，按 \n 分行，
//   每收到完整一行交给 protocol_parse() 解析。
//
// 架构位置：
//   护士站客户端 ──TCP──→ net_server(线程) ──→ protocol_parse()
//                                                    ↓
//                                             app_state_set_xxx()
//
// 粘包处理策略：
//   每个客户端维护一个 recv_buffer
//   recv 后追加到缓冲区 → 扫描 \n → 切出完整行逐条处理
//   剩余不完整的半行留在缓冲区等下次 recv 拼接

#include "net_server.h"
#include "protocol.h"       // 协议解析（JSON → app_state）
#include "../core/logger.h" // 日志记录

#include <sys/socket.h>
#include <sys/epoll.h>   // epoll 替代 select
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

// ======================== 常量 ========================

#define PORT           8888     // 监听端口
#define MAX_CLIENTS    10       // 最大客户端数
#define MAX_EVENTS     (MAX_CLIENTS + 1) // epoll 事件数组大小
#define BUF_SIZE       4096     // 每个客户端的接收缓冲区大小
#define EPOLL_TIMEOUT  1000     // epoll_wait 超时毫秒（用于定期检查退出标志）

// ======================== 客户端连接结构 ========================

typedef struct
{
    int fd;                // 客户端套接字，-1 表示空闲槽位
    char buffer[BUF_SIZE]; // 接收缓冲区（环形不覆盖，粘包拼接）
    int data_len;          // 缓冲区中已接收但未处理的数据长度
} client_slot_t;

// ======================== 全局变量 ========================

static int g_server_fd = -1;                 // 监听套接字
static int g_epoll_fd  = -1;                 // epoll 实例
static volatile int g_running = 0;           // 线程运行标志
static pthread_t g_thread;                   // 工作线程 ID
static client_slot_t g_clients[MAX_CLIENTS]; // 客户端槽位数组

// ======================== 前向声明 ========================

static void *server_thread(void *arg);

// ======================== 槽位管理 ========================

// 在客户端数组中找一个空闲槽位，返回下标，满员返回 -1
static int find_free_slot(void)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_clients[i].fd == -1)
        {
            return i;
        }
    }
    return -1;
}

// 关闭指定槽位的客户端连接，槽位重置为空闲
static void close_slot(int idx)
{
    if (idx < 0 || idx >= MAX_CLIENTS)
        return;

    // 从 epoll 中移除该 fd
    epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, g_clients[idx].fd, NULL);

    printf("[net_server] 客户端断开: fd=%d\n", g_clients[idx].fd);
    logger_write(LOG_CONNECT, "客户端断开 fd=%d", g_clients[idx].fd);
    close(g_clients[idx].fd);
    g_clients[idx].fd = -1;
    g_clients[idx].data_len = 0;
}

// ======================== 接收与粘包处理 ========================

// 处理指定槽位上的新到数据
// 核心逻辑：recv → 追加到 buffer → 按 \n 切分 → 逐行给 protocol_parse
static void handle_recv(int idx)
{
    client_slot_t *slot = &g_clients[idx];

    // 从套接字读取数据，追加到缓冲区末尾
    int space = BUF_SIZE - slot->data_len - 1; // 预留 '\0' 位置
    if (space <= 0)
    {
        // 缓冲区满了：丢弃旧数据，重置缓冲区
        logger_write(LOG_ERROR, "fd=%d 缓冲区溢出", slot->fd);
        slot->data_len = 0;
        space = BUF_SIZE - 1;
    }

    int n = recv(slot->fd, slot->buffer + slot->data_len, space, 0);

    if (n <= 0)
    {
        close_slot(idx);
        return;
    }

    slot->data_len += n;
    slot->buffer[slot->data_len] = '\0'; // 安全收尾

    // 按 \n 逐行切割
    // line_start 指向当前这一行的开头
    char *line_start = slot->buffer;
    char *newline_pos;

    while ((newline_pos = strchr(line_start, '\n')) != NULL)
    {
        // 把 \n 替换成 \0，切出一行完整的命令字符串
        *newline_pos = '\0';

        printf("[net_server] 收到命令: %s\n", line_start);

        // 交给协议解析模块 → 拿到 JSON 应答 → 发回客户端
        char *response = protocol_parse(line_start);
        if (response)
        {
            send(slot->fd, response, strlen(response), 0);
            send(slot->fd, "\n", 1, 0); // 行尾分隔符
            free(response);
        }

        // 指针跳到下一行的开头
        line_start = newline_pos + 1;
    }

    // 把剩余的不完整半行移到缓冲区开头，等下次 recv 再拼
    int remaining = slot->data_len - (line_start - slot->buffer);
    if (remaining > 0 && line_start != slot->buffer)
    {
        memmove(slot->buffer, line_start, remaining);
    }
    slot->data_len = remaining;
}

// ======================== 接受新连接 ========================

static void handle_accept(void)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(g_server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        if (errno != EINTR)
        {
            perror("[net_server] accept");
        }
        return;
    }

    int slot = find_free_slot();
    if (slot < 0)
    {
        printf("[net_server] 连接数已满，拒绝新连接\n");
        close(client_fd);
        return;
    }

    g_clients[slot].fd = client_fd;
    g_clients[slot].data_len = 0;

    // 将客户端 fd 注册到 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;       // 水平触发，监听可读
    ev.data.fd = client_fd;
    epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    printf("[net_server] 新客户端连接: IP=%s Port=%d fd=%d slot=%d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port),
           client_fd, slot);
    logger_write(LOG_CONNECT, "客户端连接 IP=%s Port=%d",
                 inet_ntoa(client_addr.sin_addr),
                 ntohs(client_addr.sin_port));
}

// ======================== epoll 主循环 ========================

static void *server_thread(void *arg)
{
    (void)arg;

    printf("[net_server] 线程启动，监听端口 %d (epoll)\n", PORT);

    struct epoll_event events[MAX_EVENTS];

    while (g_running)
    {
        // epoll_wait 阻塞等待，超时 1000ms 后检查 g_running 退出标志
        int nfds = epoll_wait(g_epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);

        if (nfds < 0)
        {
            if (errno == EINTR)
                continue; // 被信号打断，重试
            perror("[net_server] epoll_wait");
            break;
        }

        // 超时，无事件 → 回到 while 检查 g_running
        if (nfds == 0)
        {
            continue;
        }

        // 遍历所有就绪的 fd
        for (int i = 0; i < nfds; i++)
        {
            int ready_fd = events[i].data.fd;

            if (ready_fd == g_server_fd)
            {
                // 监听套接字可读 → 有新连接
                handle_accept();
            }
            else
            {
                // 客户端套接字可读 → 有数据到达
                // 找到对应的槽位
                for (int j = 0; j < MAX_CLIENTS; j++)
                {
                    if (g_clients[j].fd == ready_fd)
                    {
                        handle_recv(j);
                        break;
                    }
                }
            }
        }
    }

    // 退出：清理所有客户端连接
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (g_clients[i].fd != -1)
        {
            epoll_ctl(g_epoll_fd, EPOLL_CTL_DEL, g_clients[i].fd, NULL);
            close(g_clients[i].fd);
            g_clients[i].fd = -1;
        }
    }

    printf("[net_server] 线程退出\n");
    return NULL;
}

// ======================== 公开接口 ========================

int net_server_start(void)
{
    if (g_running)
    {
        return 0; // 已经启动了
    }

    // 1. 创建 TCP 套接字
    g_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_fd < 0)
    {
        perror("[net_server] socket");
        return -1;
    }

    // 2. 设置端口复用（避免重启时 Address already in use）
    int opt = 1;
    setsockopt(g_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 3. 绑定到 8888 端口
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡

    if (bind(g_server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[net_server] bind");
        close(g_server_fd);
        g_server_fd = -1;
        return -1;
    }

    // 4. 开始监听
    if (listen(g_server_fd, 5) < 0)
    {
        perror("[net_server] listen");
        close(g_server_fd);
        g_server_fd = -1;
        return -1;
    }

    // 5. 创建 epoll 实例
    g_epoll_fd = epoll_create1(0);
    if (g_epoll_fd < 0)
    {
        perror("[net_server] epoll_create1");
        close(g_server_fd);
        g_server_fd = -1;
        return -1;
    }

    // 6. 将监听套接字注册到 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;        // 水平触发，监听新连接
    ev.data.fd = g_server_fd;
    epoll_ctl(g_epoll_fd, EPOLL_CTL_ADD, g_server_fd, &ev);

    // 7. 初始化客户端槽位数组
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        g_clients[i].fd = -1;
        g_clients[i].data_len = 0;
    }

    // 8. 启动工作线程
    g_running = 1;
    if (pthread_create(&g_thread, NULL, server_thread, NULL) != 0)
    {
        perror("[net_server] pthread_create");
        close(g_server_fd);
        g_server_fd = -1;
        g_running = 0;
        return -1;
    }
    pthread_detach(g_thread); // 分离线程，退出时自动回收资源

    printf("[net_server] 启动成功，端口 %d\n", PORT);
    return 0;
}

void net_server_stop(void)
{
    if (!g_running)
        return;

    g_running = 0; // 通知线程退出循环

    // 关闭 epoll 和监听套接字，让 epoll_wait 返回错误从而跳出
    if (g_epoll_fd != -1)
    {
        close(g_epoll_fd);
        g_epoll_fd = -1;
    }

    if (g_server_fd != -1)
    {
        shutdown(g_server_fd, SHUT_RDWR);
        close(g_server_fd);
        g_server_fd = -1;
    }

    printf("[net_server] 已停止\n");
}
