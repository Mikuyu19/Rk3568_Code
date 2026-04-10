#include "mq2.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// 传感器采集命令，用来发送数据
static unsigned char cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

/* 打开 MQ2 对应串口 */
int mq2_open(void)
{
    int fd = open(MQ2_UART, O_RDWR);
    if (fd == -1)
    {
        perror("open mq2 fail");
        return -1;
    }

    init_tty(fd);
    printf("MQ2 uart open: %s\n", MQ2_UART);
    return fd;
}

/* 读取一次 MQ2 数据 */
int mq2_read_data(int fd, struct mq2_data *data)
{
    unsigned char buf1[9] = {0};
    int ret = 0;

    // 清空输入缓冲区，避免读取到上一次残留数据
    tcflush(fd, TCIFLUSH);

    // 1. 发送获取烟雾浓度命令
    ret = write_nbyte(fd, cmd, 9);
    if (ret != 9)
    {
        printf("write mq2 cmd fail\n");
        return -1;
    }

    // 2. 接收 9 个字节的返回数据
    ret = read_nbyte(fd, buf1, 9);
    if (ret != 9)
    {
        printf("read smoke data fail\n");
        return -1;
    }

    // 3. 判断数据帧是否正确
    if (buf1[0] == 0xFF && buf1[1] == 0x86)
    {
        data->concen = (buf1[2] << 8) | buf1[3];
    }
    else
    {
        printf("smoke frame error\n");
        return -1;
    }

    return 0;
}

/* 显示一次 MQ2 数据 */
void mq2_print_data(struct mq2_data data)
{
    printf("烟雾浓度为 %u\n", data.concen);
}
