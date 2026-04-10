#ifndef _SERIAL_PORT_H_
#define _SERIAL_PORT_H_

#include <stddef.h>

// 6818 串口所对应的文件名
#define COM2_6818 "/dev/ttySAC1"
#define COM3_6818 "/dev/ttySAC2"
#define COM4_6818 "/dev/ttySAC3"

// RK3568 串口所对应的文件名
#define COM0_3568 "/dev/ttyS0"
#define COM1_3568 "/dev/ttyS1"
#define COM3_3568 "/dev/ttyS3"
#define COM4_3568 "/dev/ttyS4"

/* 设置串口参数（初始化串口） */
void init_tty(int fd);

/* 按指定长度读满数据 */
int read_nbyte(int fd, unsigned char *buf, int len);

/* 按指定长度写满数据 */
int write_nbyte(int fd, const unsigned char *buf, int len);

#endif
