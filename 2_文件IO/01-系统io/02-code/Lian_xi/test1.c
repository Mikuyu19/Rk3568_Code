// 打印 /home/china 的子文件以及子目录的名字

#include <stdio.h>  // 标准输入输出的头文件
#include <dirent.h> // 目录操作的头文件
#include <string.h> // 字符串操作的头文件

int main()
{
    // 打开目录
    DIR *dp = opendir("/home/china");
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }
    // 读取目录项
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        // 打印目录项的名字
        printf("%s\n", entry->d_name);
    }
    // 关闭目录
    closedir(dp);
    return 0;
}