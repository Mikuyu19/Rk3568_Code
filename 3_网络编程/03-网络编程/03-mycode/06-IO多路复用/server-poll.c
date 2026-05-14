#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1;

    char buffer[BUFFER_SIZE];

    // 1. 创建 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    // 端口复用
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2. 绑定
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    // 3. 监听
    if (listen(sockfd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("poll TCP 服务器启动成功，监听端口 %d ...\n", PORT);

    // 初始化 pollfd 数组：第0个位置放服务器监听socket
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    while (1)
    {
        int ret = poll(fds, nfds, 5000); // 超时5秒

        if (ret < 0)
        {
            perror("poll");
            break;
        }
        if (ret == 0)
        {
            // printf("poll 超时...\n");
            continue;
        }

        //  1. 处理新客户端连接
        if (fds[0].revents & POLLIN)
        {
            int newfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
            if (newfd == -1)
            {
                perror("accept");
                continue;
            }

            if (nfds >= MAX_CLIENTS + 1)
            {
                printf("客户端已满，拒绝新连接！\n");
                close(newfd);
            }
            else
            {
                fds[nfds].fd = newfd;
                fds[nfds].events = POLLIN; // 监听可读事件
                fds[nfds].revents = 0;
                nfds++;

                printf("新客户端连接成功！IP: %s, 端口: %d, fd = %d\n",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port),
                       newfd);
            }
        }

        // 2. 处理已连接的客户端
        for (int i = 1; i < nfds; i++) // 从1开始，0是服务器socket
        {
            if (fds[i].revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
            {
                int n = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);

                if (n <= 0) // 客户端断开或出错
                {
                    if (n == 0)
                        printf("客户端 fd=%d 正常断开连接\n", fds[i].fd);
                    else
                        perror("recv");

                    close(fds[i].fd);

                    // 数组压缩：把最后一个有效的fd移到当前位置
                    fds[i] = fds[nfds - 1];
                    nfds--;
                    i--; // 重要！因为数组发生了变化，需要重新检查当前i位置
                }
                else
                {
                    buffer[n] = '\0';
                    printf("收到客户端(fd=%d)消息: %s\n", fds[i].fd, buffer);

                    // 回显消息给客户端
                    send(fds[i].fd, buffer, n, 0);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}