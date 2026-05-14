/*
 * Unix域套接字客户端程序
 * 功能：连接到Unix域套接字服务器，发送用户输入的消息
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h> // socket相关函数
#include <sys/un.h>     // Unix域套接字地址结构

#define UNIX_PATH "/tmp/unix_socket.sock" // Unix域套接字文件路径
#define BUFFER_SIZE 1024                  // 缓冲区大小

int main()
{
    int sockfd;                     // 套接字文件描述符
    struct sockaddr_un server_addr; // 服务器地址结构
    char buffer[BUFFER_SIZE];       // 数据缓冲区

    // 1. 创建Unix域套接字
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket error");
        exit(1);
    }

    // 2. 设置服务器地址结构
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX; // 地址族
    strncpy(server_addr.sun_path, UNIX_PATH, sizeof(server_addr.sun_path) - 1);

    // 3. 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect error");
        close(sockfd);
        exit(1);
    }

    printf("Connected to server\n");

    // 4. 循环读取用户输入到服务器
    while (1)
    {
        printf("send: ");
        fgets(buffer, sizeof(buffer) - 1, stdin);
        if (send(sockfd, buffer, strlen(buffer), 0) == -1)
        {
            perror("send error");
            break;
        }
    }

    // 5. 关闭套接字
    close(sockfd);
    return 0;
}
