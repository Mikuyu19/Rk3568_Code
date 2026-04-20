#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
    char *mp3[] = {"1.mp3", "2.mp3", "3.mp3"};
    int i = 0;
    while (1)
    {
        // 创建进程
        pid_t pid = fork();
        if (pid == 0)
        {
            // 子 负责播放音乐
            char cmd[100] = {0};                // 用来保存要执行的命令
            sprintf(cmd, "madplay %s", mp3[i]); // 拼接命令
            printf("正在播放%s\n", mp3[i]);
            // system(cmd);//执行命令
            execl("/usr/bin/madplay", "madplay", mp3[i]);
            exit(0); // 结束
        }
        else if (pid > 0)
        {
            // 父 等待子进程结束
            wait(NULL);
            printf("%s已经播放完毕\n", mp3[i]);
            i++;
            if (i == 3)
            {
                printf("当前列表已经播放完毕\n");
                break;
            }
        }
    }
}