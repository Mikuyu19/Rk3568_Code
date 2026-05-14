#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    // 创建UDP套接字：使用SOCK_DGRAM类型
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    // 配置服务器地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;                       // IPv4协议
    server_addr.sin_port = htons(8888);                     // 端口号8888
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 服务器IP

    printf("UDP客户端启动，目标服务器 127.0.0.1:8888\n");

    char buffer[1024];
    char buffer_ok[1024];
    while (1)
    {
        printf("send: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0; // 去除换行符

        // UDP发送数据：使用sendto，需要指定目标地址
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        {
            perror("sendto");
            break;
        }
        // 接收服务器回复的"ok"
        socklen_t server_addr_len = sizeof(server_addr);
        ssize_t bytes_received = recvfrom(sockfd, buffer_ok, sizeof(buffer_ok) - 1, 0,
                                          (struct sockaddr *)&server_addr, &server_addr_len);
        if (bytes_received > 0)
        {
            buffer_ok[bytes_received] = '\0';
            printf("服务器回复: %s\n", buffer_ok);
        }

        // 退出条件
        if (strcmp(buffer, "q") == 0)
        {
            printf("退出客户端\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
