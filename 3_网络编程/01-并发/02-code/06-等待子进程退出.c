#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子：%d\n",getpid());
        while(1)
        {
            printf("aaa\n");
            sleep(1);
        }
    }
    else if(pid > 0)
    {
        //父进程
        //等待子进程结束
        int wstatus;
        wait(&wstatus);       
        
        if(WIFEXITED(wstatus))//判断是否正常结束子进程
        {
            //打印退出码
            printf("exit code:%d\n",WEXITSTATUS(wstatus));
        }
        if(WIFSIGNALED(wstatus))//判断子进程是否因信号而终止
        {
            printf("signal number:%d\n",WTERMSIG(wstatus));
        }
    }    
}