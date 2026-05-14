#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
    //打开
    int fd = open("../1.txt",2);
    if(fd == -1)//失败
    {
        perror("open failed");
    }
    else
    {
        printf("ok\n");
    }
}