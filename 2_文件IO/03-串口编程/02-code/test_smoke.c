#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

// 6818 串口所对应的文件名
#define COM2_6818 "/dev/ttySAC1"
#define COM3_6818 "/dev/ttySAC2"
#define COM4_6818 "/dev/ttySAC3"

// RK3568 串口所对应的文件名
#define COM0_3568 "/dev/ttyS0"
#define COM1_3568 "/dev/ttyS1"
#define COM3_3568 "/dev/ttyS3"
#define COM4_3568 "/dev/ttyS4"

// 根据开发板实际接线修改这里
#define MQ2_UART COM0_3568

// 传感器采集命令，用来发送数据
unsigned char cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

// 保存获取到的烟雾浓度的值
unsigned int concen = 0;

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

// 按指定长度读满数据
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

void smoke(void)
{
    // 打开串口
    int fd = open(MQ2_UART, O_RDWR);
    if (fd == -1)
    {
        perror("failed open");
        return;
    }

    // 初始化串口
    init_tty(fd);

    while (1)
    {
        unsigned char buf1[9] = {0};
        int res = 0;

        // 清空输入缓冲区，避免读取到上一次残留数据
        tcflush(fd, TCIFLUSH);

        // 发送获取烟雾浓度命令
        res = write(fd, cmd, 9);
        if (res != 9)
        {
            perror("write cmd fail");
            close(fd);
            return;
        }

        // 接收 9 个字节的返回数据
        res = read_nbyte(fd, buf1, 9);
        if (res != 9)
        {
            printf("read smoke data fail\n");
            sleep(1);
            continue;
        }

        // 判断数据帧是否正确
        if (buf1[0] == 0xFF && buf1[1] == 0x86)
        {
            concen = (buf1[2] << 8) | buf1[3];
            printf("烟雾浓度为 %u\n", concen);
        }
        else
        {
            printf("smoke frame error\n");
        }

        sleep(1);
    }

    close(fd);
}

int main(void)
{
    smoke();
    return 0;
}
