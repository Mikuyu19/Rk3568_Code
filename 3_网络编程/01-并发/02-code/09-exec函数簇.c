#include <stdio.h>
#include <unistd.h>
int main()
{
#if 0
    int ret = execl("/mnt/hgfs/CS2612/三阶段/01-并发/01-进程/02-code/08-test","./08-test","3","4",NULL);
    if(ret == -1)
    {
        perror("execl failed");
        return -1;
    }
    printf("ret == %d\n",ret);
#endif

#if 1
    execlp("08-test", "./08-test", "3", "4", NULL); // 路径一定要加环境变量
#endif

#if 0
    char *argv[] = {"./08-test", "3", "4", NULL};
    int ret = execv("/mnt/c/Users/Zelda/Desktop/Rk3568_Code/3_网络编程/01-并发/02-code/08-test", argv);
    if (ret == -1)
    {
        perror("Error");
        return -1;
    }
    printf("ret = %d\n", ret);
    return 0;
#endif
}