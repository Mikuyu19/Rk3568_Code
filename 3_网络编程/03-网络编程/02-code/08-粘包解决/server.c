#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

/*
    初始化服务端
    参数
        port:端口的字符串
    返回值
        返回服务端的套接字
*/
int init_server_socket(char *port)
{
    // 创建套接字
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
    {
        perror("socket failed");
        return -1;
    }

    // bind绑定一个主机的网络地址
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;         // ipv4
    sa.sin_port = htons(atoi(port)); // 指定端口号  把端口号主机字节h序转化为网络字节序n
    // inet_aton("172.6.1.209",&sa.sin_addr);//指定ip 将点分十进制ip地址转化为二进制网络字节序ip
    sa.sin_addr.s_addr = inet_addr("0.0.0.0");
    /*
        ip还可以写0.0.0.0
        表示监听服务器上所有的ipv4的地址，也就是说
        无论是从哪一个ip发送过来的数据，只要是目标端口和监听的端口相匹配
        服务器都处理数据包
    */
    if (bind(server_sock, (struct sockaddr *)&sa, (socklen_t)sizeof(sa)))
    {
        perror("bind");
        close(server_sock);
        return -1;
    }
    // listen:让套机子进入监听模式
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
        printf("input:%s <port>\n", argv[0]);
        return -1;
    }

    int server_sock = init_server_socket(argv[1]);

    printf("正在等待客户端连接\n");
    // accept:用于server接收一个来自客户端的连接请求
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
    printf("client ip:%s,port:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // 通信
    char buf[1024];
    while (1)
    {
        // 接收数据长度
        int length = 0;
        int rbytes = recv(client_fd, &length, 4, 0);
        if (rbytes < 0)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }
        // 接收数据
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
                printf("client is offline\n");
                exit(EXIT_FAILURE);
            }
            total_recv += rbytes;
        }
        printf(" %s\n", buf);
        sleep(1);
    }

    close(client_fd);
    close(server_sock);
}