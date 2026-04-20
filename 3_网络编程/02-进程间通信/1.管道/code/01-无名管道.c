// #include <stdio.h>
// #include <sys/types.h>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// int main()
// {
//     //创建无名管道
//     int pipefd[2];//保存无名管道读和写的文件描述符
//     pipe(pipefd);
//     //创建子进程
//     pid_t pid = fork();
//     if(pid == 0)
//     {
//         //子 写
//         close(pipefd[0]);//关闭子进程的读端
//         char buf[100] = {0};//保存输入的数据
//         fgets(buf,sizeof(buf)-1,stdin);//从键盘输入
//         write(pipefd[1],buf,strlen(buf));//写数据
//         close(pipefd[1]);//关闭子进程的写端
//     }
//     else if(pid > 0)
//     {
//         //父 读
//         close(pipefd[1]);//关闭父进程的写端
//         char buf[100] = {0};//用来保存读取到的数据
//         read(pipefd[0],buf,sizeof(buf)-1);//读取数据
//         printf("read_buf:%s\n",buf);
//         close(pipefd[0]);//关闭父进程的读端
//     }

// }

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
int main()
{
    //创建无名管道
    int pipefd[2];//保存无名管道读和写的文件描述符
    pipe(pipefd);
    //创建子进程
    pid_t pid = fork();
    if(pid == 0)
    {
        //子 写
        close(pipefd[0]);//关闭父进程的读端
        write(pipefd[1],"abcdef",6);
        exit(0);

    }
    else if(pid > 0)
    {
        //父 读
        close(pipefd[0]);//关闭父进程的读端

        int wstatus;
        wait(&wstatus);
        if(WIFSIGNALED(wstatus))//判断子进程是否被信号终止
        {
            printf("子进程被信号终止%d\n",WTERMSIG(wstatus));
        }
    }

}