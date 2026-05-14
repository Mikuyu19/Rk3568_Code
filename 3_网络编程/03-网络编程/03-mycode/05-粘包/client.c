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
    初始化客户端套接字并连接服务器
    参数：
        ip: 服务器IP地址
        port: 服务器端口号
    返回值：
        成功返回客户端套接字，失败返回-1
*/
int init_client_socket(char *ip, char *port)
{
    // 创建TCP套接字
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock == -1)
    {
        perror("socket failed");
        return -1;
    }

    // 设置服务器地址结构
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;                // IPv4协议
    sa.sin_port = htons(atoi(port));        // 端口号转换为网络字节序
    inet_aton(ip, &sa.sin_addr);            // IP地址转换

    printf("正在连接服务器...\n");
    // 连接到服务器
    if (connect(client_sock, (struct sockaddr *)&sa, (socklen_t)sizeof(sa)))
    {
        perror("connect");
        close(client_sock);
        return -1;
    }
    printf("成功连接服务器\n");

    return client_sock;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("使用方法:%s <服务器IP> <端口号>\n", argv[0]);
        return -1;
    }

    // 初始化客户端套接字并连接服务器
    int client_sock = init_client_socket(argv[1], argv[2]);

    char *buff = "hello,abc";

    while (1)
    {
        // 计算要发送的数据长度
        int length = strlen(buff);

        // 分配内存：4字节存储长度 + 实际数据长度
        char *pbuff = malloc(sizeof(char) * (length + 4));

        // 第一步：将数据长度拷贝到缓冲区前4字节
        memcpy(pbuff, &length, 4);

        // 第二步：将实际数据拷贝到缓冲区后面
        memcpy(pbuff + 4, buff, length);

        // 一次性发送：长度(4字节) + 数据内容
        // 这样服务端就能先读取长度，再根据长度读取完整数据，避免粘包
        int wbytes = send(client_sock, pbuff, length + 4, 0);
        if (wbytes < 0)
        {
            perror("send");
            free(pbuff);
            exit(EXIT_FAILURE);
        }

        usleep(1000); // 延时1毫秒
        free(pbuff);  // 释放内存
    }

    // 关闭套接字
    close(client_sock);
    return 0;
}
