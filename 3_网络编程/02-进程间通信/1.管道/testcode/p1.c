#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int main()
{
    mkfifo("/tmp/myfifo", 0666);
    int fd = open("/tmp/myfifo", O_WRONLY);

    char buf[128];
    while (1)
    {
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = 0;
        write(fd, buf, strlen(buf) + 1);
        if (buf[0] == 'q' && buf[1] == 0)
            break;
    }
    close(fd);
    return 0;
}