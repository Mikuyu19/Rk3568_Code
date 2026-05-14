// 练习：判断一个文件的类型，文件名的名字从命令行传入
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct stat sb; // 保存属性
    stat(argv[1], &sb);
    if (S_ISREG(sb.st_mode))
    {
        printf("%s 是普通文件\n", argv[1]);
    }
    else if (S_ISDIR(sb.st_mode))
    {
        printf("%s 是目录\n", argv[1]);
    }
    else
    {
        printf("%s 是其他类型的文件\n", argv[1]);
    }
}