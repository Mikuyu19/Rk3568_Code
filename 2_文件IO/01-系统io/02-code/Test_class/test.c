#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    int fd = open("../1.txt", O_RDONLY);
    if (fd == -1)
    {
        // 如果打开失败（返回 -1），使用 perror 打印错误信息
        perror("打开上一层目录的 1.txt 失败");
    }
    else
    {
        // 如果打开成功，打印 ok
        printf("ok\n");
        close(fd);
    }

    return 0;
}