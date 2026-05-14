// ========================
// nurse_client.c —— 护士站测试客户端
// ========================
//
// 最简单的叫号测试工具，用法：
//
//   call 012 张三 1      叫号：号码 姓名 诊室(1或2)
//   wait 013 李四 1 014 王五 2  批量推送队列
//   doctor 1 王医生      换医生：诊室 姓名
//   notice 请排队等候     发公告
//   clear               关公告
//   ping                心跳
//   help / quit

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 8888
#define BUF_SIZE 4096

static int g_sockfd = -1;

// ======================== 网络收发 ========================

static int connect_server(const char *ip, int port)
{
    g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sockfd < 0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(g_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(g_sockfd);
        g_sockfd = -1;
        return -1;
    }
    printf("已连接 %s:%d\n\n", ip, port);
    return 0;
}

// 发送 JSON 并收应答
static void send_json(const char *json)
{
    if (g_sockfd < 0)
    {
        printf("未连接\n");
        return;
    }

    char buf[BUF_SIZE];
    snprintf(buf, sizeof(buf), "%s\n", json);
    send(g_sockfd, buf, strlen(buf), 0);

    memset(buf, 0, sizeof(buf));
    int n = recv(g_sockfd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
    {
        printf("  -> %s\n", buf);
    }
    else if (n == 0)
    {
        printf("  服务器断开\n");
        close(g_sockfd);
        g_sockfd = -1;
    }
}

// ======================== 命令 ========================

static void cmd_call(int argc, char **argv)
{
    // call <号码> <姓名> <诊室>
    // 诊室写 1 或 2 即可，自动拼成 1诊室
    if (argc < 4)
    {
        printf("用法: call <号码> <姓名> <诊室(1或2)>\n");
        printf("示例: call 012 张三 1\n");
        return;
    }
    char room[16];
    snprintf(room, sizeof(room), "%s诊室", argv[3]);

    char json[BUF_SIZE];
    snprintf(json, sizeof(json),
             "{\"cmd\":\"call\",\"number\":\"%s\",\"name\":\"%s\",\"room\":\"%s\"}",
             argv[1], argv[2], room);
    send_json(json);
}

static void cmd_doctor(int argc, char **argv)
{
    // doctor <诊室(1或2)> <姓名>
    if (argc < 3)
    {
        printf("用法: doctor <诊室(1或2)> <姓名>\n");
        printf("示例: doctor 1 王医生\n");
        return;
    }
    int idx = atoi(argv[1]) - 1; // 1→0, 2→1
    if (idx < 0 || idx > 1)
    {
        printf("诊室只能是 1 或 2\n");
        return;
    }

    char room[16];
    snprintf(room, sizeof(room), "%s诊室", argv[1]);

    char json[BUF_SIZE];
    snprintf(json, sizeof(json),
             "{\"cmd\":\"doctor\",\"index\":%d,\"name\":\"%s\",\"room\":\"%s\"}",
             idx, argv[2], room);
    send_json(json);
}

static void cmd_notice(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("用法: notice <文本>\n");
        printf("示例: notice 请排队等候\n");
        return;
    }
    char json[BUF_SIZE];
    snprintf(json, sizeof(json), "{\"cmd\":\"notice\",\"text\":\"%s\"}", argv[1]);
    send_json(json);
}

static void cmd_wait(int argc, char **argv)
{
    // wait <号码> <姓名> <1或2> [号码 姓名 1或2 ...]
    // 每三个参数一组，可以一次传多组
    if (argc < 4 || (argc - 1) % 3 != 0)
    {
        printf("用法: wait <号码> <姓名> <1或2> [号码 姓名 1或2 ...]\n");
        printf("示例: wait 013 李四 1 014 王五 2\n");
        return;
    }

    char json[BUF_SIZE];
    int pos = 0;
    pos += snprintf(json + pos, sizeof(json) - pos,
                    "{\"cmd\":\"wait\",\"queue\":[");

    int count = (argc - 1) / 3;
    for (int i = 0; i < count; i++)
    {
        char room[16];
        snprintf(room, sizeof(room), "%s诊室", argv[i * 3 + 3]);

        pos += snprintf(json + pos, sizeof(json) - pos,
                        "%s{\"number\":\"%s\",\"name\":\"%s\",\"room\":\"%s\"}",
                        i > 0 ? "," : "",
                        argv[i * 3 + 1], // 号码
                        argv[i * 3 + 2], // 姓名
                        room);           // 诊室(自动拼 1诊室)
    }

    snprintf(json + pos, sizeof(json) - pos, "]}");
    send_json(json);
}

static void cmd_clear(void) { send_json("{\"cmd\":\"clear_notice\"}"); }
static void cmd_ping(void) { send_json("{\"cmd\":\"ping\"}"); }

// ======================== 帮助 ========================

static void show_help(void)
{
    printf("命令列表:\n");
    printf("  call   <号码> <姓名> <1或2>    叫号\n");
    printf("  wait   <号码> <姓名> <1或2> ... 批量推送队列\n");
    printf("  doctor <1或2> <姓名>           换医生\n");
    printf("  notice <文本>                  发公告\n");
    printf("  clear                         关公告\n");
    printf("  ping                          心跳\n");
    printf("  help / quit\n");
    printf("\n示例:\n");
    printf("  call 012 张三 1\n");
    printf("  wait 013 李四 1 014 王五 2\n");
    printf("  doctor 1 王医生\n");
    printf("  notice 请排队等候\n");
    printf("\n");
}

// ======================== 主函数 ========================

int main(int argc, char **argv)
{
    const char *ip = (argc >= 2) ? argv[1] : DEFAULT_IP;
    int port = (argc >= 3) ? atoi(argv[2]) : DEFAULT_PORT;

    if (connect_server(ip, port) < 0)
        return 1;
    show_help();

    char line[BUF_SIZE];
    while (1)
    {
        printf("nurse> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL)
            break;

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0')
            continue;

        char *tokens[32];
        int n = 0;
        char *save, *tok = strtok_r(line, " ", &save);
        while (tok && n < 32)
        {
            tokens[n++] = tok;
            tok = strtok_r(NULL, " ", &save);
        }

        const char *cmd = tokens[0];

        if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0)
        {
            break;
        }
        else if (strcmp(cmd, "help") == 0)
        {
            show_help();
        }
        else if (strcmp(cmd, "call") == 0)
        {
            cmd_call(n, tokens);
        }
        else if (strcmp(cmd, "wait") == 0)
        {
            cmd_wait(n, tokens);
        }
        else if (strcmp(cmd, "doctor") == 0)
        {
            cmd_doctor(n, tokens);
        }
        else if (strcmp(cmd, "notice") == 0)
        {
            cmd_notice(n, tokens);
        }
        else if (strcmp(cmd, "clear") == 0)
        {
            cmd_clear();
        }
        else if (strcmp(cmd, "ping") == 0)
        {
            cmd_ping();
        }
        else
        {
            printf("未知命令 '%s'，输入 help 查看帮助\n", cmd);
        }
    }

    if (g_sockfd >= 0)
        close(g_sockfd);
    printf("已退出\n");
    return 0;
}
