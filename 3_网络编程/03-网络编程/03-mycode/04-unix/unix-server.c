/*
 * Unix域套接字服务器程序
 * 功能：创建Unix域套接字服务器，接收并显示客户端发送的消息
 */

#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UNIX_PATH "/tmp/unix_socket.sock" // Unix域套接字文件路径
#define BUFFER_SIZE 1024                  // 缓冲区大小

int main()
{
    int sockfd, connfd;                          // 监听套接字和连接套接字
    struct sockaddr_un server_addr, client_addr; // 服务器和客户端地址结构

    // 0. 删除可能存在的旧套接字
    unlink(UNIX_PATH);

    // 1. 创建Unix域套接字
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    // 2. 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX; // 地址
    strncpy(server_addr.sun_path, UNIX_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. 绑定套接字到指定路径
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind error");
        close(sockfd);
        exit(1);
    }

    // 4. 监听连接请求
    if (listen(sockfd, 5) == -1)
    {
        perror("listen error");
        close(sockfd);
        return 1;
    }

    printf("Server listening on %s\n", UNIX_PATH);

    // 5. 接收客户端连接
    socklen_t client_len; // 客户端地址结构长度
    client_len = sizeof(client_addr);
    connfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
    if (connfd < 0)
    {
        perror("accept error");
        close(sockfd);
        exit(1);
    }
    printf("Client connected\n");

    // 6. 循环接收客户端数据
    char buffer[BUFFER_SIZE]; // 数据缓冲区
    ssize_t n;                // 读取的字节数
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        n = read(connfd, buffer, BUFFER_SIZE - 1);
        if (n <= 0)
        {
            if (n == 0)
            {
                printf("Client disconnected\n"); // 客户端正常断开
            }
            else
            {
                perror("read error"); // 读取错误
            }
            break;
        }
        printf("Received: %s", buffer); // 打印接收到的数据
    }

    // 7. 关闭套接字并删除套接字文件
    close(connfd);
    close(sockfd);
    unlink(UNIX_PATH); // 清理套接字文件

    return 0;
}