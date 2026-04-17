#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子：%d\n",getpid());
        printf("父：%d\n",getppid());
        // while(1)
        // {
        //     printf("aaaa\n");
        //     sleep(1);
        // }
    }
    else if(pid > 0)
    {
        printf("pid = %d\n",pid);//打一下pid的值 看一下是否跟子进程id一致
        //父进程
        // while(1)
        // {
        //     printf("bbbb\n");
        //     sleep(1);
        // }
    }    
}