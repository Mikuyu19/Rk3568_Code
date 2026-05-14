#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define FIFOPATH "/home/china/1.fifo"

// 播放列表
char *video[] = {"1.mp4", "2.mp4", "3.mp4"};
int num_videos = 3;
int current_index = 0; // 当前播放歌曲下标

// 状态记录
int volume_value = 50; // 初始音量
int is_mute = 0;       // 静音标志：0-不静音，1-静音

void video_play()
{
    // 1. 创建并打开有名管道 (用于父进程向子进程mplayer发送命令)
    if (access(FIFOPATH, F_OK) == -1)
    {
        mkfifo(FIFOPATH, 0666);
    }

    int fdfifo = open(FIFOPATH, O_RDWR);
    if (fdfifo == -1)
    {
        perror("open FIFOPATH fail");
        exit(0);
    }

    // 2. 创建无名管道 (用于父进程读取mplayer的输出，如当前时间等)
    int fds[2]; // 0 读, 1 写
    if (pipe(fds))
    {
        perror("pipe fail");
        exit(0);
    }

    // 3. 创建子进程启动 mplayer
    pid_t pid = fork();
    if (pid == 0) // 子进程
    {
        // 将标准输出重定向到无名管道，mplayer的打印信息会进入无名管道
        dup2(fds[1], STDOUT_FILENO);

        char fifoname[128] = {0};
        sprintf(fifoname, "file=%s", FIFOPATH);
        // 启动mplayer，默认播放第一首
        execlp("mplayer", "mplayer", "-slave", "-quiet", "-input", fifoname, "-zoom", "-x", "800", "-y", "480", video[current_index], NULL);
        perror("execlp fail");
        exit(0);
    }
    else if (pid > 0) // 父进程：作为控制端
    {
        char cmd[128] = {0};
        printf("播放器已启动！\n控制按键：\n [p]暂停/继续 [n]下一首 [v]上一首 \n [+]音量加 [-]音量减 [m]静音/取消 \n [a]快进 [b]快退 [q]退出\n");

        while (1)
        {
            char c = getchar();
            if (c == '\n')
                continue; // 过滤掉回车键带来的影响

            switch (c)
            {
            case 'p': // 暂停/继续
                write(fdfifo, "pause\n", strlen("pause\n"));
                printf("=> 切换暂停/继续状态\n");
                break;
            case 'n': // 下一首
                current_index = (current_index + 1) % num_videos;
                sprintf(cmd, "loadfile %s\n", video[current_index]);
                write(fdfifo, cmd, strlen(cmd));
                printf("=> 切换下一首: %s\n", video[current_index]);
                break;
            case 'v': // 上一首
                current_index = (current_index - 1 + num_videos) % num_videos;
                sprintf(cmd, "loadfile %s\n", video[current_index]);
                write(fdfifo, cmd, strlen(cmd));
                printf("=> 切换上一首: %s\n", video[current_index]);
                break;
            case '+': // 音量增加
                if (volume_value < 100)
                    volume_value += 10;
                sprintf(cmd, "volume %d 1\n", volume_value);
                write(fdfifo, cmd, strlen(cmd));
                printf("=> 当前音量: %d\n", volume_value);
                break;
            case '-': // 音量减少
                if (volume_value > 0)
                    volume_value -= 10;
                sprintf(cmd, "volume %d 1\n", volume_value);
                write(fdfifo, cmd, strlen(cmd));
                printf("=> 当前音量: %d\n", volume_value);
                break;
            case 'm': // 静音切换
                is_mute = !is_mute;
                sprintf(cmd, "mute %d\n", is_mute);
                write(fdfifo, cmd, strlen(cmd));
                printf("=> %s\n", is_mute ? "已开启静音" : "已解除静音");
                break;
            case 'a': // 快进
                write(fdfifo, "seek +5\n", strlen("seek +5\n"));
                break;
            case 'b': // 快退
                write(fdfifo, "seek -5\n", strlen("seek -5\n"));
                break;
            case 'q': // 退出
                write(fdfifo, "quit\n", strlen("quit\n"));
                wait(NULL); // 回收子进程，避免僵尸进程
                close(fdfifo);
                close(fds[0]);
                close(fds[1]);
                printf("=> 播放器退出\n");
                exit(0);
            }
        }
    }
}

int main()
{
    video_play();
    return 0;
}