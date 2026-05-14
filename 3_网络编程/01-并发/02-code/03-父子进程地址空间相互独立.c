#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int global = 0;
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        global = 100;
        while(1)
        {
            printf("子：%d %p\n",global,&global);
            sleep(1);
        }
    }
    else if(pid > 0)
    {
        //父进程
        while(1)
        {
            printf("父：%d %p\n",global,&global);
            sleep(1);
        }

    }    
}