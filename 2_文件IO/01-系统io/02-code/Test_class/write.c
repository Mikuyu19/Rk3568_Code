#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    int fd;

    unsigned int color = 0x00ff0000;
    int width = 1024; // 屏幕宽度
    int height = 600; // 屏幕高度
    int i;

    fd = open("/dev/fb0", O_WRONLY);
    if (fd == -1)
    {
        perror("打开屏幕设备 /dev/fb0 失败");
        return -1;
    }

    for (i = 0; i < width * height; i++)
    {
        int ret = write(fd, &color, 4);
        if (ret == -1)
        {
            perror("写入数据失败");
            break;
        }
    }

    // 3. 关闭屏幕
    close(fd);

    return 0;
}