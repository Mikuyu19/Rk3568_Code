#include "serial_port.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

/* 设置串口参数（初始化串口） */
void init_tty(int fd)
{
    // 1. 先有保存串口属性的结构体
    struct termios termios_new;

    // 2. 先清空该结构体
    bzero(&termios_new, sizeof(termios_new));

    // 3. 激活选项，设置为原始模式
    cfmakeraw(&termios_new);

    // 4. 激活本地连接和接收使能
    termios_new.c_cflag |= CLOCAL | CREAD;

    // 5. 设置波特率
    cfsetispeed(&termios_new, B9600);
    cfsetospeed(&termios_new, B9600);

    // 6. 设置数据位为 8 位
    termios_new.c_cflag &= ~CSIZE;
    termios_new.c_cflag |= CS8;

    // 7. 设置无奇偶校验
    termios_new.c_cflag &= ~PARENB;

    // 8. 设置一位停止位
    termios_new.c_cflag &= ~CSTOPB;

    // 9. 设置最少字符和等待时间
    // 这里保持和 test_gy39、test_smoke 一样的配置。
    termios_new.c_cc[VTIME] = 10;
    termios_new.c_cc[VMIN] = 1;

    // 10. 清空输入缓冲区
    tcflush(fd, TCIFLUSH);

    // 11. 立即生效
    if (tcsetattr(fd, TCSANOW, &termios_new) != 0)
    {
        printf("Setting the serial failed!\n");
    }
}

/* 按指定长度读满数据 */
int read_nbyte(int fd, unsigned char *buf, int len)
{
    int total = 0;
    int ret = 0;

    while (total < len)
    {
        ret = read(fd, buf + total, len - total);
        if (ret < 0)
        {
            perror("read");
            return -1;
        }
        if (ret == 0)
        {
            return -1;
        }
        total += ret;
    }

    return total;
}

/* 按指定长度写满数据 */
int write_nbyte(int fd, const unsigned char *buf, int len)
{
    int total = 0;
    int ret = 0;

    while (total < len)
    {
        ret = write(fd, buf + total, len - total);
        if (ret < 0)
        {
            perror("write");
            return -1;
        }
        if (ret == 0)
        {
            return -1;
        }
        total += ret;
    }

    return total;
}
