#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8888
#define MAX_EVENTS 200 // 一次 epoll_wait 最多返回的事件数
#define MAX_CLIENTS 1000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd, epfd;
    struct sockaddr_in server_addr;
    struct epoll_event ev, events[MAX_EVENTS];

    char buffer[BUFFER_SIZE];

    // 1. 创建监听 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 设置非阻塞（推荐，即使先用 LT 模式也建议设置）
    // fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(sockfd, 10);

    // 2. 创建 epoll 实例
    epfd = epoll_create1(0);
    if (epfd == -1)
    {
        perror("epoll_create1");
        exit(1);
    }

    // 3. 把监听 socket 添加到 epoll 中
    ev.events = EPOLLIN; // 水平触发
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    printf("epoll TCP 服务器启动成功，监听端口 %d\n", PORT);

    while (1)
    {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); // -1 表示永久阻塞

        if (nfds < 0)
        {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;

            // 1. 有新客户端连接
            if (fd == sockfd)
            {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int newfd = accept(sockfd, (struct sockaddr *)&client_addr, &len);

                if (newfd == -1)
                    continue;

                // 设置新连接为非阻塞（推荐）
                // fcntl(newfd, F_SETFL, fcntl(newfd, F_GETFL) | O_NONBLOCK);

                ev.events = EPOLLIN;
                ev.data.fd = newfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, newfd, &ev);

                printf("新客户端连接 fd=%d, IP=%s\n",
                       newfd, inet_ntoa(client_addr.sin_addr));
            }
            // 2. 已连接客户端有数据或断开
            else
            {
                int n = recv(fd, buffer, sizeof(buffer) - 1, 0);

                if (n <= 0) // 客户端断开
                {
                    if (n == 0)
                        printf("客户端 fd=%d 断开连接\n", fd);
                    else
                        perror("recv");

                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL); // 从 epoll 中删除
                }
                else
                {
                    buffer[n] = '\0';
                    printf("收到客户端(fd=%d): %s\n", fd, buffer);

                    // 回显
                    send(fd, buffer, n, 0);
                }
            }
        }
    }

    close(sockfd);
    close(epfd);
    return 0;
}