// 练习 2：计算给定目录下面，有多少普通文件和目录文件

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

void count_files(const char *path)
{
    int reg_count = 0; // 普通文件计数器
    int dir_count = 0; // 目录文件计数器
    // 打开目录
    DIR *dp = opendir(path);
    if (dp == NULL)
    {
        perror("opendir");
        return;
    }
    // 读取目录项
    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        // 构建完整的路径
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        // 获取文件状态
        struct stat st;
        // 处理 stat 错误
        if (stat(full_path, &st) == -1)
        {
            perror("stat");
            continue;
        }
        // 判断文件类型
        if (S_ISREG(st.st_mode))
        {
            reg_count++;
        }
        else if (S_ISDIR(st.st_mode))
        {
            dir_count++;
        }
    }
    // 打印结果
    printf("普通文件数量: %d\n", reg_count);
    printf("目录文件数量: %d\n", dir_count);
    // 关闭目录
    closedir(dp);
}

int main()
{
    count_files("/home/china");
    return 0;
}