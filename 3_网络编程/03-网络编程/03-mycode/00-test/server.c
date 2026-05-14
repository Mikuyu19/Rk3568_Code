
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MAX_CLIENTS 10 // 最大客户端连接数
#define BUF_SIZE 1024  // 缓冲区大小

// 客户端信息结构体
typedef struct
{
    int fd;      // 客户端socket文件描述符
    char ip[16]; // 客户端IP地址
    int port;    // 客户端端口号
    int online;  // 在线状态：1在线 0离线
    int id;      // 客户端编号
} client_info;

client_info clients[MAX_CLIENTS];                  // 客户端列表数组
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥锁，保护客户端列表
int client_count = 0;                              // 当前客户端数量

// 发送在线客户端列表给指定客户端
void send_client_list(int fd)
{
    char buf[BUF_SIZE] = {0};
    char line[128];

    // 构建表头
    sprintf(buf, "\n编号\tIP\t\t端口\t在线状态\n");
    strcat(buf, "----------------------------------------\n");

    // 加锁访问共享的客户端列表
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].online)
        {
            sprintf(line, "%d\t%s\t%d\t%d\n",
                    clients[i].id, clients[i].ip, clients[i].port, clients[i].online);
            strcat(buf, line);
        }
    }
    pthread_mutex_unlock(&mutex);

    // 发送列表数据给客户端
    send(fd, buf, strlen(buf), 0);
}

// 处理私聊消息：pm 编号 消息内容
void handle_private_msg(int sender_fd, char *msg)
{
    int target_id;
    char content[900];
    char send_buf[BUF_SIZE];

    // 解析命令格式：pm 目标编号 消息内容
    if (sscanf(msg, "pm %d %[^\n]", &target_id, content) != 2)
    {
        char err[] = "格式错误！使用: pm 编号 消息内容\n";
        send(sender_fd, err, strlen(err), 0);
        return;
    }

    pthread_mutex_lock(&mutex);
    int found = 0;
    int sender_id = -1;

    // 查找发送者的ID
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].fd == sender_fd && clients[i].online)
        {
            sender_id = clients[i].id;
            break;
        }
    }

    // 查找目标客户端并转发消息
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].id == target_id && clients[i].online)
        {
            snprintf(send_buf, BUF_SIZE, "[私聊] 用户%d: %s\n", sender_id, content);
            send(clients[i].fd, send_buf, strlen(send_buf), 0);

            // 给发送者确认消息
            char confirm[] = "消息已发送\n";
            send(sender_fd, confirm, strlen(confirm), 0);
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    // 目标用户不存在或不在线
    if (!found)
    {
        char err[] = "目标用户不在线或不存在\n";
        send(sender_fd, err, strlen(err), 0);
    }
}

// 客户端处理线程函数
void *client_handler(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);
    char buf[BUF_SIZE];

    while (1)
    {
        memset(buf, 0, BUF_SIZE);
        int len = recv(client_fd, buf, BUF_SIZE - 1, 0);

        // 客户端断开连接
        if (len <= 0)
        {
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].fd == client_fd)
                {
                    printf("客户端 %d (%s:%d) 断开连接\n",
                           clients[i].id, clients[i].ip, clients[i].port);
                    clients[i].online = 0; // 标记为离线
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
            close(client_fd);
            break;
        }

        // 处理接收到的消息
        buf[len] = '\0';
        if (buf[len - 1] == '\n')
            buf[len - 1] = '\0';

        // 根据命令类型分发处理
        if (strcmp(buf, "list") == 0)
        {
            send_client_list(client_fd);
        }
        else if (strncmp(buf, "pm ", 3) == 0)
        {
            handle_private_msg(client_fd, buf);
        }
    }

    return NULL;
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t tid;

    // 初始化客户端列表
    memset(clients, 0, sizeof(clients));

    // 创建TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    // 设置地址重用，避免TIME_WAIT状态占用端口
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 监听所有网卡
    server_addr.sin_port = htons(8888);       // 监听8888端口

    // 绑定地址
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // 开始监听，最大等待队列为5
    if (listen(server_fd, 5) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("服务器启动，监听端口 8888...\n");

    // 主循环：接受客户端连接
    while (1)
    {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1)
        {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&mutex);
        // 查找空闲位置
        int idx = -1;
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (!clients[i].online)
            {
                idx = i;
                break;
            }
        }

        // 服务器已满，拒绝连接
        if (idx == -1)
        {
            pthread_mutex_unlock(&mutex);
            char msg[] = "服务器已满\n";
            send(client_fd, msg, strlen(msg), 0);
            close(client_fd);
            continue;
        }

        // 记录客户端信息
        clients[idx].fd = client_fd;
        strcpy(clients[idx].ip, inet_ntoa(client_addr.sin_addr));
        clients[idx].port = ntohs(client_addr.sin_port);
        clients[idx].online = 1;
        clients[idx].id = idx + 1;

        printf("新客户端连接: ID=%d, %s:%d\n",
               clients[idx].id, clients[idx].ip, clients[idx].port);

        pthread_mutex_unlock(&mutex);

        // 为该客户端创建处理线程
        int *arg = malloc(sizeof(int));
        *arg = client_fd;
        pthread_create(&tid, NULL, client_handler, arg);
        pthread_detach(tid); // 分离线程，自动回收资源
    }

    close(server_fd);
    return 0;
}
