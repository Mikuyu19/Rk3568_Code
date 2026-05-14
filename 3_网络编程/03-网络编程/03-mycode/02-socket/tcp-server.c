#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("bind");
        close(sockfd);
        return 1;
    }
    // Start listening for connections
    if (listen(sockfd, 5) == -1)
    {
        perror("listen");
        close(sockfd);
        return 1;
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd == -1)
        {
            perror("accept");
            continue;
        }
        printf("%s[%d]已成功连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        while (1)
        {
            char buffer[1024];
            // 使用read和write函数来接收和发送数据
            // ssize_t bytes_received = read(client_sockfd, buffer, sizeof(buffer) - 1);
            // 使用recv函数来接收数据
            ssize_t bytes_received = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received > 0)
            {
                buffer[bytes_received] = '\0';
                printf("Received message: %s\n", buffer);
            }
            else if (bytes_received == 0)
            {
                printf("Client disconnected.\n");
                break;
            }
        }

        close(client_sockfd);
    }
    close(sockfd);
    return 0;
}