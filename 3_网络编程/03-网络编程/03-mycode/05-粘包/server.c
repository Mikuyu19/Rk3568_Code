#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/*
    初始化服务端套接字
    参数：
        port: 端口号字符串
    返回值：
        成功返回服务端套接字，失败返回-1
*/
int init_server_socket(char *port)
{
    // 创建TCP套接字
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket failed");
        return -1;
    }

    // 设置服务器地址结构
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;                // IPv4协议
    sa.sin_port = htons(atoi(port));        // 端口号转换为网络字节序
    sa.sin_addr.s_addr = inet_addr("0.0.0.0"); // 监听所有网络接口

    // 绑定套接字到指定地址和端口
    if (bind(server_sock, (struct sockaddr *)&sa, (socklen_t)sizeof(sa)))
    {
        perror("bind");
        close(server_sock);
        return -1;
    }

    // 设置套接字为监听模式，最大连接队列为5
    if (listen(server_sock, 5))
    {
        perror("listen");
        close(server_sock);
        return -1;
    }
    return server_sock;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("使用方法:%s <端口号>\n", argv[0]);
        return -1;
    }

    // 初始化服务端套接字
    int server_sock = init_server_socket(argv[1]);

    printf("正在等待客户端连接...\n");

    // 接受客户端连接
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    int client_fd = accept(server_sock, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd == -1)
    {
        perror("accept");
        close(server_sock);
        return -1;
    }

    printf("客户端已连接\n");
    printf("客户端 IP:%s, 端口:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 接收数据缓冲区
    char buf[1024];

    while (1)
    {
        // 第一步：先接收4字节的数据长度
        int length = 0;
        int rbytes = recv(client_fd, &length, 4, 0);
        if (rbytes < 0)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        else if (rbytes == 0)
        {
            printf("客户端已断开连接\n");
            break;
        }

        // 清空缓冲区
        memset(buf, 0, sizeof(buf));

        // 第二步：根据接收到的长度，循环接收完整数据
        // 这样可以避免粘包问题，确保接收到完整的一条消息
        int total_recv = 0;
        while (total_recv < length)
        {
            rbytes = recv(client_fd, buf + total_recv, length - total_recv, 0);
            if (rbytes < 0)
            {
                perror("recv");
                exit(EXIT_FAILURE);
            }
            else if (rbytes == 0)
            {
                printf("客户端已断开连接\n");
                exit(EXIT_FAILURE);
            }
            total_recv += rbytes; // 累加已接收的字节数
        }

        printf("收到数据: %s\n", buf);
        sleep(1);
    }

    // 关闭套接字
    close(client_fd);
    close(server_sock);
    return 0;
}
