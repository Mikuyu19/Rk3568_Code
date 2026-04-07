#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

// 递归打印全路径
void print_all_paths(const char *base_path)
{
    DIR *dp = opendir(base_path);
    if (dp == NULL)
        return; // 遇到没有权限或者打不开的目录，直接返回

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

        // 打印拼接好的包含路径的文件名
        printf("%s\n", next_path);

        // 如果是目录，继续递归
        {
            struct stat st;
            if (stat(next_path, &st) == 0 && S_ISDIR(st.st_mode))
            {
                print_all_paths(next_path);
            }
        }
    }
    closedir(dp);
}

int main()
{
    printf("开始递归打印当前目录及子目录的文件:\n");
    print_all_paths(".");
    return 0;
}