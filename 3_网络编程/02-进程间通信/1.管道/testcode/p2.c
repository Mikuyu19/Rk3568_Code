#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main()
{
    mkfifo("/tmp/myfifo", 0666);
    int fd = open("/tmp/myfifo", O_RDONLY);

    char buf[128];
    while (1)
    {
        read(fd, buf, sizeof(buf));
        printf("收到: %s\n", buf);
        if (buf[0] == 'q' && buf[1] == 0)
            break;
    }
    close(fd);
    return 0;
}