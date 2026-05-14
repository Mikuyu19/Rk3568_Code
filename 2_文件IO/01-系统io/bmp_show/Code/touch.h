#ifndef _TOUCH_H_
#define _TOUCH_H_

//放头文件
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//全局变量的声明
extern int read_x;
extern int read_y;
//函数声明
void get_xy();
#endif