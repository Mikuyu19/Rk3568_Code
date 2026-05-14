#ifndef _GY39_H_
#define _GY39_H_

#include "serial_port.h"

// 根据开发板实际接线修改这里
#define GY39_UART COM1_3568

struct gy39_data
{
    float light;
    float temperature;
    float pressure;
    float humidity;
    float altitude;
};

/* 打开 GY39 对应串口 */
int gy39_open(void);

/* 读取一次 GY39 数据 */
int gy39_read_data(int fd, struct gy39_data *data);

/* 显示一次 GY39 数据 */
void gy39_print_data(struct gy39_data data);

#endif
