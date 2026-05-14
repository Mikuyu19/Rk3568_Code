// ========================
// udp_discover.c —— UDP 设备发现
// ========================
//
// 让护士站能自动发现叫号屏，不用手动输 IP。
//
// 工作流程：
//   1. 本模块监听 UDP 9999 端口
//   2. 护士站广播 "WHO_IS_CALL_SCREEN" 到局域网
//   3. 本模块收到后回复 "CALL_SCREEN,room=1,port=8888"
//   4. 护士站收到回复后自动连接 TCP 8888 端口
//
// 注意：setsockopt SO_BROADCAST 用于接收广播包，
//       SO_REUSEADDR 允许多个进程复用端口。

#include "udp_discover.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define DISCOVER_PORT 9999 // UDP 监听端口
#define BUF_SIZE 256       // 接收缓冲区

// ======================== 全局变量 ========================

static int g_sock_fd = -1;         // UDP 套接字
static volatile int g_running = 0; // 线程运行标志
static pthread_t g_thread;         // 工作线程

// ======================== 线程函数 ========================

static void *discover_thread(void *arg)
{
    (void)arg;

    printf("[udp_discover] 线程启动，监听 UDP 端口 %d\n", DISCOVER_PORT);

    char buf[BUF_SIZE];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (g_running)
    {
        memset(buf, 0, sizeof(buf));

        // 阻塞等待广播包（超时 1 秒，方便退出检查）
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(g_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        int n = recvfrom(g_sock_fd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&from_addr, &from_len);

        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue; // 超时，检查 g_running 标志
            }
            perror("[udp_discover] recvfrom");
            continue;
        }

        buf[n] = '\0';

        // 去掉尾部换行符（echo/nc 发数据时会带 \n）
        buf[strcspn(buf, "\r\n")] = '\0';

        printf("[udp_discover] 收到广播: %s 来自 %s:%d\n",
               buf,
               inet_ntoa(from_addr.sin_addr),
               ntohs(from_addr.sin_port));

        // 判断是否为护士站发现请求
        if (strcmp(buf, "WHO_IS_CALL_SCREEN") == 0)
        {
            // 构造应答：设备类型 + 诊室 + TCP 端口
            const char *reply = "CALL_SCREEN,room=1,port=8888";

            int ret = sendto(g_sock_fd, reply, strlen(reply), 0,
                   (struct sockaddr *)&from_addr, from_len);
            if (ret < 0) {
                perror("[udp_discover] sendto 失败");
            } else {
                printf("[udp_discover] 已回复: %s\n", reply);
            }
        }
    }

    printf("[udp_discover] 线程退出\n");
    return NULL;
}

// ======================== 公开接口 ========================

int udp_discover_start(void)
{
    if (g_running)
    {
        return 0; // 已经启动
    }

    // 1. 创建 UDP 套接字
    g_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock_fd < 0)
    {
        perror("[udp_discover] socket");
        return -1;
    }

    // 2. 允许接收广播包
    int broadcast = 1;
    setsockopt(g_sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    // 3. 设置端口复用
    int opt = 1;
    setsockopt(g_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 4. 绑定 9999 端口
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DISCOVER_PORT);
    addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡

    if (bind(g_sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("[udp_discover] bind");
        close(g_sock_fd);
        g_sock_fd = -1;
        return -1;
    }

    // 5. 启动工作线程
    g_running = 1;
    if (pthread_create(&g_thread, NULL, discover_thread, NULL) != 0)
    {
        perror("[udp_discover] pthread_create");
        close(g_sock_fd);
        g_sock_fd = -1;
        g_running = 0;
        return -1;
    }
    pthread_detach(g_thread); // 分离，退出自动回收

    printf("[udp_discover] 启动成功，端口 %d\n", DISCOVER_PORT);
    return 0;
}

void udp_discover_stop(void)
{
    if (!g_running)
        return;

    g_running = 0; // 通知线程退出

    if (g_sock_fd != -1)
    {
        close(g_sock_fd);
        g_sock_fd = -1;
    }

    printf("[udp_discover] 已停止\n");
}

// 模拟 UDP 发现
// echo "WHO_IS_CALL_SCREEN" | nc -u -b 255.255.255.255 9999
// 叫号屏会回复: CALL_SCREEN,room=1,port=8888