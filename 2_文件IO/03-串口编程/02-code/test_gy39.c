#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>

// 6818 串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"

// RK3568 串口所对应的文件名
#define COM0_3568 "/dev/ttyS0"
#define COM1_3568 "/dev/ttyS1"
#define COM3_3568 "/dev/ttyS3"
#define COM4_3568 "/dev/ttyS4"

// 根据开发板实际接线修改这里
#define GY39_UART COM1_3568

float data[5] = {0};

// 定义命令，设置模块的工作方式
unsigned char cmd1[3] = {0xa5, 0x81, 0x26}; // 获取光照强度
unsigned char cmd2[3] = {0xa5, 0x82, 0x27}; // 获取温湿度气压海拔
unsigned char cmd3[3] = {0xa5, 0x83, 0x28}; // 获取光照+温湿度气压海拔

/* 设置串口参数（初始化串口） */
void init_tty(int fd)
{
    // 1. 先有保存串口属性的结构体
    struct termios termios_new;

    // 2. 先清空该结构体
    bzero(&termios_new, sizeof(termios_new));

    // 3. 激活原始模式
    cfmakeraw(&termios_new);

    // 4. 激活本地连接和接收使能
    termios_new.c_cflag |= CLOCAL | CREAD;

    // 5. 设置波特率为 9600
    cfsetispeed(&termios_new, B9600);
    cfsetospeed(&termios_new, B9600);

    // 6. 设置数据位为 8 位
    termios_new.c_cflag &= ~CSIZE;
    termios_new.c_cflag |= CS8;

    // 7. 设置无奇偶校验
    termios_new.c_cflag &= ~PARENB;

    // 8. 设置一位停止位
    termios_new.c_cflag &= ~CSTOPB;

    // 9. 设置超时和最少接收字节数
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

// GY39 模块
void GY39(void)
{
    // 1. 打开串口
    int ttyfd = open(GY39_UART, O_RDWR);
    if (-1 == ttyfd)
    {
        perror("open ttyfd fail");
        return;
    }

    // 2. 初始化串口
    init_tty(ttyfd);

    while (1)
    {
        unsigned char rbuf1[9] = {0};
        unsigned char rbuf2[15] = {0};
        unsigned int Lux = 0;
        unsigned int T = 0;
        unsigned int P = 0;
        unsigned int HUM = 0;
        int H = 0;
        int ret = 0;

        // 清空输入缓冲区，避免上一次数据残留
        tcflush(ttyfd, TCIFLUSH);

        // 3. 发送命令，获取光照强度
        ret = write(ttyfd, cmd1, 3);
        if (ret != 3)
        {
            perror("write cmd1 fail");
            close(ttyfd);
            return;
        }

        // 4. 接收 9 个字节的光照数据帧
        ret = read_nbyte(ttyfd, rbuf1, 9);
        if (ret != 9)
        {
            printf("read light data fail\n");
            sleep(1);
            continue;
        }

        if (rbuf1[0] == 0x5A && rbuf1[1] == 0x5A && rbuf1[2] == 0x15)
        {
            Lux = (rbuf1[4] << 24) | (rbuf1[5] << 16) | (rbuf1[6] << 8) | rbuf1[7];
            data[0] = Lux / 100.0f;
        }
        else
        {
            printf("light frame error\n");
            sleep(1);
            continue;
        }

        // 5. 发送命令，获取温度、气压、湿度、海拔
        ret = write(ttyfd, cmd2, 3);
        if (ret != 3)
        {
            perror("write cmd2 fail");
            close(ttyfd);
            return;
        }

        // 6. 接收 15 个字节的数据帧
        ret = read_nbyte(ttyfd, rbuf2, 15);
        if (ret != 15)
        {
            printf("read env data fail\n");
            sleep(1);
            continue;
        }

        if (rbuf2[0] == 0x5A && rbuf2[1] == 0x5A && rbuf2[2] == 0x45)
        {
            // 温度
            T = (rbuf2[4] << 8) | rbuf2[5];
            data[1] = T / 100.0f;

            // 气压
            P = (rbuf2[6] << 24) | (rbuf2[7] << 16) | (rbuf2[8] << 8) | rbuf2[9];
            data[2] = P / 100.0f;

            // 湿度
            HUM = (rbuf2[10] << 8) | rbuf2[11];
            data[3] = HUM / 100.0f;

            // 海拔
            H = (rbuf2[12] << 8) | rbuf2[13];
            data[4] = (float)H;
        }
        else
        {
            printf("env frame error\n");
            sleep(1);
            continue;
        }

        // 7. 显示所有数据
        printf("light = %.2f lux\n", data[0]);
        printf("T = %.2f C\n", data[1]);
        printf("P = %.2f Pa\n", data[2]);
        printf("HUM = %.2f %%\n", data[3]);
        printf("H = %.0f m\n", data[4]);
        printf("-------------------------\n");

        sleep(1);
    }

    close(ttyfd);
}

int main(void)
{
    GY39();
    return 0;
}
