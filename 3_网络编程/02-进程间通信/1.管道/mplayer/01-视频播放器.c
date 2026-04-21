#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#define FIFOPATH "/home/china/1.fifo"
char *video[] = {"1.mp4", "2.mp4", "3.mp4"};
// 取一首歌
int i = 0;    // i为歌曲的下标

void video_play()
{
    // 打开有名管道 //写
    int fdfifo = open(FIFOPATH, O_RDWR);
    if (fdfifo == -1)
    {
        perror("open FIFOPATH fail");
        exit(0);
    }

    int fds[2];//0 读 1 写
    //打开无名管道 //读
    if(pipe(fds))
    {
        perror("");
        exit(0);
    }

    pid_t pid = fork();
    if (pid == 0) // 子
    {
        //把标准输出重定向到无名管道写端 ->结果不会打印到终端 全都写入到无名管道
        dup2(fds[1],STDOUT_FILENO);

        char fifoname[128] = {0};
        sprintf(fifoname, "file=%s", FIFOPATH);
        execlp("mplayer", "mplayer", "-slave", "-quiet", "-input", fifoname, "-zoom", "-x", "800", "-y", "480", video[i], NULL);
    }
    else if (pid > 0) // 父进程
    {
        //把fds[0] 设置成非阻塞
        char cmd[50] = {0};
        while (1)
        {
            char c = getchar();
            switch (c)
            {
            case 'a': // 快进
                write(fdfifo, "seek +5\n", strlen("seek +5\n"));
                break;
            case 'b': // 快退
                write(fdfifo, "seek -5\n", strlen("seek -5\n"));
                break;
            case 'c':
                 write(fdfifo, "get_time_length\n", strlen("get_time_length\n"));
                 char buff[100] = {0};
                 //读取mplayer响应
                 float total_time = 0;
                 while(1)
                 {
                    //清空buffer
                    memset(buff,0,sizeof(buff));
                    //从无名管道读取数据
                    read(fds[0],buff,sizeof(buff)-1);

                    //检查是否以ANS_开头的响应
                    if(strncmp(buff,"ANS_",4) == 0)
                    {
                        sscanf(buff,"ANS_LENGTH=%f",&total_time);
                        break;
                    }
                 }
                 printf("总时长为%.2f\n",total_time);
                 break;
            case 'q': // 退出
                write(fdfifo, "quit\n", strlen("quit\n"));
                wait(NULL); // 回收子进程的资源
                exit(0);    // 退出程序
            }
        }
    }

}

int main()
{
    // 音视频开始播放
    video_play();
}