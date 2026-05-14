// 练习：收到ctrl c和ctrl \ 信号，改变默认行为，变成打印收到了 % d信号(% d为信号的值)
#if 0
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum)
{
    printf("收到了%d信号\n", signum);
}

int main()
{
    signal(SIGINT, handler);  // Ctrl+C (信号2)
    signal(SIGQUIT, handler); // Ctrl+\ (信号3)
    printf("程序运行中，按 Ctrl+C 或 Ctrl+\\ 测试\n");
    while (1)
    {
        sleep(1);
    }
    return 0;
}
#endif

#if 0
// 例1：优雅退出程序
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int running = 1;

void handler(int sig)
{
    running = 0; // 设置标志位
}

int main()
{
    signal(SIGINT, handler);

    while (running)
    {
        printf("工作中...\n");
        sleep(1);
    }

    printf("清理资源，退出\n");
    return 0;
}

#endif

#if 0
// 例2：父进程回收子进程
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

void sigchld_handler(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ; // 回收所有子进程
}

int main()
{
    signal(SIGCHLD, sigchld_handler);

    if (fork() == 0)
    {
        sleep(2);
        exit(0);
    }

    while (1)
        sleep(1);
}

#endif

#if 1
//  例3：定时器

#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void alarm_handler(int sig)
{
    printf("起床了\n");
    alarm(3);
}

int main()
{
    signal(SIGALRM, alarm_handler);
    alarm(3); // 3秒后触发

    while (1)
        ;
}

#endif