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
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;         // IPv4协议
    sa.sin_port = htons(8888);       // 端口号8888
    sa.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口

    // 绑定地址到套接字（UDP服务器必须bind）
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("UDP服务器启动，监听端口 8888...\n");

    // UDP无需listen和accept，直接接收数据
    while (1)
    {
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // UDP接收数据：使用recvfrom，会返回发送方地址
        ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                                          (struct sockaddr *)&client_addr, &client_addr_len);
        if (bytes_received > 0)
        {
            buffer[bytes_received] = '\0';
            printf("收到来自 %s[%d] 的消息: %s\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port),
                   buffer);

            // 回复给客户端”ok”
            sendto(sockfd, "ok", strlen("ok"), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
        }
        else if (bytes_received == -1)
        {
            perror("recvfrom");
            break;
        }
    }

    close(sockfd);
    return 0;
}
