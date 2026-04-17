#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子:%d\n",getpid());
        exit(0);
    }
    else if(pid > 0)
    {
        //父进程
        printf("父:%d\n",getpid());
        while(1);

    }    
}