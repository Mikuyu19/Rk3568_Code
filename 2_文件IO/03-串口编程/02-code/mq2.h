#ifndef _MQ2_H_
#define _MQ2_H_

#include "serial_port.h"

// 根据开发板实际接线修改这里
#define MQ2_UART COM0_3568

struct mq2_data
{
    unsigned int concen;
};

/* 打开 MQ2 对应串口 */
int mq2_open(void);

/* 读取一次 MQ2 数据 */
int mq2_read_data(int fd, struct mq2_data *data);

/* 显示一次 MQ2 数据 */
void mq2_print_data(struct mq2_data data);

#endif
