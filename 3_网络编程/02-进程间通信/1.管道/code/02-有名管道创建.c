#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    //创建管道
    int ret = mkfifo("/home/china/1.fifo", 0777);
    if(ret == -1)
    {
        if(errno == EEXIST)//已经存在
        {
            printf("文件已存在，🙅不要创建\n");
        }
        else
        {
            perror("mkfifo failed");
            return -1;
        }
    }

    //已经存在就直接做后面的事情
    int fd = open("/home/china/1.fifo",2);

}