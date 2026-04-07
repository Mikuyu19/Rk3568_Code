#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main()
{
    //改变工作路径为/home/china
    #if 0
    int ret = chdir("/home/china");
    if(ret == -1)
    {
        perror("chdir failed");
        return -1;
    }
    #endif 

    #if 1
    int fd = open("/home/china",O_RDONLY);//打开路径 一定为只读
    int ret = fchdir(fd);//切换路径
    if(ret == -1)
    {
        perror("fchdir failed");
        return -1;
    }
    #endif 

    //打印当前的工作路径
    char buf[100] = {0};
    char *p = getcwd(buf,sizeof(buf)-1);
    if(p == NULL)
    {
        perror("getcwd fail");
        return -1;
    }
    printf("buf:%s\n",buf);
    printf("p:%s\n",p);
    system("./a.out");
}