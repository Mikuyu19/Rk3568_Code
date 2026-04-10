#include "gy39.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// 定义命令，设置模块的工作方式
static unsigned char cmd1[3] = {0xa5, 0x81, 0x26}; // 获取光照强度
static unsigned char cmd2[3] = {0xa5, 0x82, 0x27}; // 获取温湿度气压海拔

/* 打开 GY39 对应串口 */
int gy39_open(void)
{
    int fd = open(GY39_UART, O_RDWR);
    if (fd == -1)
    {
        perror("open gy39 fail");
        return -1;
    }

    init_tty(fd);
    printf("GY39 uart open: %s\n", GY39_UART);
    return fd;
}

/* 读取一次 GY39 数据 */
int gy39_read_data(int fd, struct gy39_data *data)
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
    tcflush(fd, TCIFLUSH);

    // 1. 发送命令，获取光照强度
    ret = write_nbyte(fd, cmd1, 3);
    if (ret != 3)
    {
        printf("write gy39 cmd1 fail\n");
        return -1;
    }

    // 2. 接收 9 个字节的光照数据帧
    ret = read_nbyte(fd, rbuf1, 9);
    if (ret != 9)
    {
        printf("read light data fail\n");
        return -1;
    }

    if (rbuf1[0] == 0x5A && rbuf1[1] == 0x5A && rbuf1[2] == 0x15)
    {
        Lux = (rbuf1[4] << 24) | (rbuf1[5] << 16) | (rbuf1[6] << 8) | rbuf1[7];
        data->light = Lux / 100.0f;
    }
    else
    {
        printf("light frame error\n");
        return -1;
    }

    // 3. 发送命令，获取温度、气压、湿度、海拔
    ret = write_nbyte(fd, cmd2, 3);
    if (ret != 3)
    {
        printf("write gy39 cmd2 fail\n");
        return -1;
    }

    // 4. 接收 15 个字节的数据帧
    ret = read_nbyte(fd, rbuf2, 15);
    if (ret != 15)
    {
        printf("read env data fail\n");
        return -1;
    }

    if (rbuf2[0] == 0x5A && rbuf2[1] == 0x5A && rbuf2[2] == 0x45)
    {
        // 温度
        T = (rbuf2[4] << 8) | rbuf2[5];
        data->temperature = T / 100.0f;

        // 气压
        P = (rbuf2[6] << 24) | (rbuf2[7] << 16) | (rbuf2[8] << 8) | rbuf2[9];
        data->pressure = P / 100.0f;

        // 湿度
        HUM = (rbuf2[10] << 8) | rbuf2[11];
        data->humidity = HUM / 100.0f;

        // 海拔
        H = (rbuf2[12] << 8) | rbuf2[13];
        data->altitude = (float)H;
    }
    else
    {
        printf("env frame error\n");
        return -1;
    }

    return 0;
}

/* 显示一次 GY39 数据 */
void gy39_print_data(struct gy39_data data)
{
    printf("light = %.2f lux\n", data.light);
    printf("T = %.2f C\n", data.temperature);
    printf("P = %.2f Pa\n", data.pressure);
    printf("HUM = %.2f %%\n", data.humidity);
    printf("H = %.0f m\n", data.altitude);
}
