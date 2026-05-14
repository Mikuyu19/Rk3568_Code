#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main()
{
    // 求上一次的umask
    // int pre_umask_value = umask(0); // 2
    // printf("pre_umask_value:%o\n", pre_umask_value);

    // 创建一个新的文件
    int fd = open("./2.txt", 2 | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("open fail");
        return 0;
    }

    printf("ok\n");

}