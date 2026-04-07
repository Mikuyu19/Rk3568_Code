#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 

int main()
{
    const char *target_dir = "/home/china";
    char buf[256] = {0};

    // 1. 改变当前进程的工作路径
    printf("将工作路径切换至: %s\n", target_dir);
    int ret = chdir(target_dir);

    if (ret == -1)
    {
        perror("切换目录失败");
        return -1;
    }
    else
    {
        printf("切换目录成功！\n\n");
    }
    // 2. 获取并打印当前工作目录
    char *p = getcwd(buf, sizeof(buf) - 1);

    if (p == NULL)
    {
        perror("getcwd 获取当前工作目录失败");
        return -1;
    }

    printf("最终确认：当前进程的工作路径是 [%s]\n", p);

    return 0;
}