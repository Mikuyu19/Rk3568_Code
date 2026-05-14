#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define MAX_CLIENTS 50
#define PORT 8888

int main()
{
    // 创建套接字 sockfd
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket error");
        return -1;
    }

    // 设置端口复用（避免服务器重启时 Address already in use）
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 绑定
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        return -1;
    }

    // 监听
    if (listen(sockfd, 5) < 0)
    {
        perror("listen error");
        return -1;
    }
    printf("服务器启动，监听端口: %d\n", PORT);

    int clientfds[MAX_CLIENTS] = {0}; // 保存所有的客户端的fd
    int client_count = 0;             // 客户端的数量
    int maxfd = sockfd;               // 初始化最大的文件描述符

    while (1)
    {
        // 1. 定义一个集合
        fd_set readfds; // 可读文件描述符集合

        // 2. 清空集合
        FD_ZERO(&readfds);

        // 3. 把你需要监听的fd加入到可读集合
        FD_SET(sockfd, &readfds); // 添加服务器的fd

        for (int i = 0; i < client_count; i++)
        {
            FD_SET(clientfds[i], &readfds); // 添加全部客户端的fd
        }

        // 4. 设置超时时间 5s
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // 5. select
        int ret = select(maxfd + 1, &readfds, NULL, NULL, &tv);

        // 6. 处理结果
        if (ret > 0) // 检查具体的fd
        {
            // 如果 sockfd 就绪，代表有客户端向你发起连接
            if (FD_ISSET(sockfd, &readfds))
            {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int newclientfd = accept(sockfd, (struct sockaddr *)&client_addr, &len);

                if (newclientfd > 0)
                {
                    printf("新客户端连接: IP=%s, Port=%d, fd=%d\n",
                           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), newclientfd);

                    if (client_count < MAX_CLIENTS)
                    {
                        clientfds[client_count] = newclientfd;
                        client_count++; // 客户端的数量+1

                        // 更新maxfd
                        if (maxfd < newclientfd)
                        {
                            maxfd = newclientfd;
                        }
                    }
                    else
                    {
                        printf("达到最大连接数，拒绝新连接\n");
                        close(newclientfd);
                    }
                }
            }

            // 遍历客户端集合，处理通信
            for (int i = 0; i < client_count; i++)
            {
                if (FD_ISSET(clientfds[i], &readfds)) // 判断clientfds[i]是否就绪
                {
                    char buf[1024] = {0};
                    // 有客户端给你发送数据
                    int n = recv(clientfds[i], buf, sizeof(buf) - 1, 0);

                    if (n > 0)
                    {
                        printf("收到客户端(fd=%d)数据: %s\n", clientfds[i], buf);
                        // 回显数据给客户端
                        send(clientfds[i], buf, n, 0);
                    }
                    else if (n == 0)
                    {
                        // recv 返回值0 代表有客户端退出
                        printf("客户端(fd=%d)已退出\n", clientfds[i]);
                        close(clientfds[i]); // 关闭退出的套接字

                        // 从数组中剔除这个fd，并将后面的元素往前移
                        for (int j = i; j < client_count - 1; j++)
                        {
                            clientfds[j] = clientfds[j + 1];
                        }
                        client_count--; // 客户端数量-1
                        i--;            // 修正索引：因为后面的元素往前填补了，i必须回退1，否则会漏掉检查下一个fd

                        // 因为可能刚刚关闭的那个就是 maxfd，所以需要重新计算最大的 fd
                        maxfd = sockfd; // 先重置为基础的监听套接字
                        for (int k = 0; k < client_count; k++)
                        {
                            if (clientfds[k] > maxfd)
                            {
                                maxfd = clientfds[k];
                            }
                        }
                    }
                    else
                    {
                        perror("recv error");
                    }
                }
            }
        }
        else if (ret == 0)
        {
            // 如果觉得每 5 秒打印一次太烦人，可以把这行注释掉
            // printf("超时\n");
        }
        else
        {
            perror("select error");
            break;
        }
    }

    close(sockfd);
    return 0;
}