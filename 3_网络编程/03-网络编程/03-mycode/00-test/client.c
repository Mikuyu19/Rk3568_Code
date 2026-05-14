#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024     // 缓冲区大小

int sockfd;               // 客户端socket文件描述符
int running = 1;          // 运行标志：1运行 0退出

// 接收线程：循环接收服务器消息并显示
void *recv_thread(void *arg) {
    char buf[BUF_SIZE];

    while (running) {
        memset(buf, 0, BUF_SIZE);
        int len = recv(sockfd, buf, BUF_SIZE - 1, 0);

        // 服务器断开连接
        if (len <= 0) {
            printf("\n服务器断开连接\n");
            running = 0;
            break;
        }

        // 显示接收到的消息
        printf("%s", buf);
        fflush(stdout);
    }

    return NULL;
}

// 发送线程：读取用户输入并发送给服务器
void *send_thread(void *arg) {
    char buf[BUF_SIZE];

    // 显示命令帮助
    printf("\n命令说明:\n");
    printf("  list          - 查看在线用户列表\n");
    printf("  pm 编号 消息  - 发送私聊消息\n");
    printf("  quit          - 退出\n\n");

    while (running) {
        printf("> ");
        fflush(stdout);

        // 读取用户输入
        if (fgets(buf, BUF_SIZE, stdin) == NULL) {
            break;
        }

        // 处理退出命令
        if (strncmp(buf, "quit", 4) == 0) {
            running = 0;
            break;
        }

        // 发送消息给服务器
        send(sockfd, buf, strlen(buf), 0);
    }

    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t recv_tid, send_tid;

    // 创建TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    // 配置服务器地址
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // 连接本地服务器
    server_addr.sin_port = htons(8888);                     // 服务器端口8888

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("已连接到服务器\n");

    // 创建接收和发送线程
    pthread_create(&recv_tid, NULL, recv_thread, NULL);
    pthread_create(&send_tid, NULL, send_thread, NULL);

    // 等待发送线程结束（用户输入quit）
    pthread_join(send_tid, NULL);
    running = 0;

    close(sockfd);
    printf("已退出\n");

    return 0;
}
