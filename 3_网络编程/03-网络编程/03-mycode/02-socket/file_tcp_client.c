#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>

/**
 * 发送文件函数
 * @param sockfd 已连接的套接字描述符
 * @param filename 要发送的文件路径
 * @return 成功返回0，失败返回-1
 */
int send_file(int sockfd, const char *filename)
{
    // 以二进制只读模式打开文件
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        perror("fopen");
        return -1;
    }

    // 获取文件大小：使用stat函数获取文件信息
    struct stat st;
    if (stat(filename, &st) == -1)
    {
        perror("stat");
        fclose(fp);
        return -1;
    }
    off_t filesize = st.st_size; // 文件大小（字节数）

    // 发送文件名的长度（4字节整数）
    int filename_len = strlen(filename);
    send(sockfd, &filename_len, sizeof(filename_len), 0);

    // 发送文件名字符串（不包含'\0'）
    send(sockfd, filename, filename_len, 0);

    // 发送文件大小（8字节off_t类型）
    send(sockfd, &filesize, sizeof(filesize), 0);

    // 循环发送文件内容
    char buffer[4096]; // 每次读取4KB
    size_t bytes_read;
    // fread返回实际读取的字节数，读到文件末尾返回0
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0)
    {
        // 将读取到的数据发送给服务器
        send(sockfd, buffer, bytes_read, 0);
    }

    fclose(fp);
    printf("文件发送完成: %s (%ld bytes)\n", filename, filesize);
    return 0;
}

int main(int argc, char *argv[])
{
    // 检查命令行参数：需要提供文件路径
    if (argc != 2)
    {
        printf("用法: %s <文件路径>\n", argv[0]);
        return 1;
    }

    // 创建TCP套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        return 1;
    }

    // 配置服务器地址结构体
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;           // IPv4协议
    server_addr.sin_port = htons(8888);         // 端口号8888（转为网络字节序）
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr); // 服务器IP地址

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sockfd);
        return 1;
    }

    printf("已连接到服务器\n");

    // 调用发送文件函数
    send_file(sockfd, argv[1]);

    // 关闭套接字
    close(sockfd);
    return 0;
}
