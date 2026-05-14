#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

/**
 * 接收文件函数
 * @param client_sockfd 客户端连接的套接字描述符
 * @return 成功返回0，失败返回-1
 */
int recv_file(int client_sockfd)
{
    // 切换工作路径：不能与传输的文件在同一路径下
    // 尝试切换到 ./received 目录，如果不存在则创建
    if (chdir("./received") == -1)
    {
        if (mkdir("./received", 0755) == -1)
        {
            perror("mkdir");
            return -1;
        }
        chdir("./received");
    }

    // 接收文件名的长度大小（4字节整数）
    int filename_len;
    recv(client_sockfd, &filename_len, sizeof(filename_len), 0);

    // 接收文件名字符串
    char filename[256];
    recv(client_sockfd, filename, filename_len, 0);
    filename[filename_len] = '\0'; // 手动添加字符串结束符

    // 接收文件大小（8字节off_t类型）
    off_t filesize;
    recv(client_sockfd, &filesize, sizeof(filesize), 0);

    printf("接收文件: %s (%ld bytes)\n", filename, filesize);

    // 以二进制写模式打开文件
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    // 循环接收文件内容
    char buffer[4096];        // 每次接收4KB
    off_t total_received = 0; // 已接收的总字节数
    ssize_t bytes_received;

    // 循环条件：接收的总字节数 < 文件大小
    while (total_received < filesize)
    {
        // 接收数据
        bytes_received = recv(client_sockfd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0)
        {
            // 连接断开或出错
            break;
        }
        // 将接收到的数据写入文件
        fwrite(buffer, 1, bytes_received, fp);
        // 累加已接收字节数
        total_received += bytes_received;
    }

    fclose(fp);
    printf("文件接收完成: %s (%ld/%ld bytes)\n", filename, total_received, filesize);
    return 0;
}

int main()
{
    // 创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    // 设置地址重用选项，避免"Address and Port already in use"错误
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    // 配置服务器地址结构体
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;         // IPv4协议
    sa.sin_port = htons(8888);       // 端口号8888（转为网络字节序）
    sa.sin_addr.s_addr = INADDR_ANY; // 监听所有网络接口（0.0.0.0）

    // 绑定地址到套接字
    if (bind(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("bind");
        close(sockfd);
        return 1;
    }

    // 开始监听连接请求，最多5个排队连接
    if (listen(sockfd, 5) == -1)
    {
        perror("listen");
        close(sockfd);
        return 1;
    }

    printf("服务器启动，监听端口 8888...\n");

    // 主循环：持续接受客户端连接
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        // 接受客户端连接
        int client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sockfd == -1)
        {
            perror("accept");
            continue;
        }

        printf("%s[%d] 已连接\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 调用接收文件函数
        recv_file(client_sockfd);

        // 关闭客户端连接
        close(client_sockfd);
    }

    // 关闭监听套接字（实际上这行代码不会执行到）
    close(sockfd);
    return 0;
}
