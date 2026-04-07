// 练习 4：求一个目录的大小（递归所有普通文件大小之和）
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

// 递归计算目录大小
long long calculate_directory_size(const char *base_path)
{
    long long total_size = 0; // 用于累加目录大小
    // 打开目录
    DIR *dp = opendir(base_path);
    if (dp == NULL)
    {
        return 0; // 遇到没有权限或者打不开的目录，直接返回0
    }

    struct dirent *ep;
    char next_path[1024]; // 用来拼接完整的路径

    while ((ep = readdir(dp)) != NULL)
    {
        if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
        {
            continue;
        }

        // 拼接路径：base_path + "/" + 当前文件名
        snprintf(next_path, sizeof(next_path), "%s/%s", base_path, ep->d_name);

        // 获取文件状态
        struct stat st;
        if (stat(next_path, &st) == -1)
        {
            perror("stat");
            continue;
        }

        // 如果是普通文件，累加大小
        if (S_ISREG(st.st_mode))
        {
            total_size += st.st_size;
        }
        // 如果是目录，递归计算子目录大小
        else if (S_ISDIR(st.st_mode))
        {
            total_size += calculate_directory_size(next_path);
        }
    }
    closedir(dp);
    return total_size;
}

int main()
{
    const char *directory_path = "."; // 可以修改为需要计算大小的目录路径
    long long size = calculate_directory_size(directory_path);
    printf("目录 '%s' 的总大小: %lld 字节\n", directory_path, size);
    return 0;
}