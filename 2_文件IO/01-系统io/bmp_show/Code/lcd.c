#include "lcd.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int lcd_fd = -1;    // 屏幕fd
int *plcd = NULL;   // plcd指针

// lcd打开
void lcd_open()
{
    // 打开屏幕
    lcd_fd = open("/dev/fb0", 2);
    if (lcd_fd == -1)
    {
        perror("open lcd fail");
        return;
    }
    // 映射
    plcd = mmap(NULL, 1024 * 600 * 4, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, lcd_fd, 0);
    if (plcd == NULL)
    {
        perror("mmap fail");
        return;
    }
}

// lcd关闭
void lcd_close()
{
    // 解除隐射
    munmap(plcd, 1024 * 600 * 4);

    // 关闭屏幕
    close(lcd_fd);
}

/*
    画点子函数
    i:x轴
    j:y轴
*/
void lcd_draw_point(int i, int j, unsigned int color)
{
    if (i >= 0 && i < 1024 && j >= 0 && j < 600)
        *(plcd + 1024 * j + i) = color;
}

void bmp_display(int x0, int y0, char *bmp_path)
{
    unsigned char a, r, g, b;
    unsigned int color;
    int i = 0;

    // 1.打开图片
    int bmp_fd = open(bmp_path, O_RDWR);
    if (bmp_fd < 0)
    {
        perror("open bmp failed");
        return;
    }
    // 2.获取图片的宽高色深

    int width = 0;
    lseek(bmp_fd, 0x12, SEEK_SET);
    read(bmp_fd, &width, 4);
    int height = 0;
    lseek(bmp_fd, 0x16, SEEK_SET);
    read(bmp_fd, &height, 4);
    int depth = 0;
    lseek(bmp_fd, 0x1C, SEEK_SET);
    read(bmp_fd, &depth, 2);

    // 3.获取像素数组的大小
    int line_bytes = abs(width) * depth / 8;
    int laizi = 0;

    if (line_bytes % 4)
    {
        laizi = 4 - line_bytes % 4;
    }
    line_bytes += laizi;
    int total_bytes = line_bytes * abs(height);

    char pix[total_bytes]; // 用来保存读到的像素数组的数据

    // 4.跳转光标到像素数组的位置
    lseek(bmp_fd, 54, SEEK_SET);

    //  5.读取像素数组的数据
    read(bmp_fd, pix, total_bytes); // 读取数据放到pix中

    //  6.解析像素数组的数据
    for (int y = 0; y < abs(height); y++) // 行
    {
        for (int x = 0; x < abs(width); x++) // 列
        {
            b = pix[i++];
            g = pix[i++];
            r = pix[i++];
            if (depth == 32)
                a = pix[i++];
            else if (depth == 24)
                a = 0;
            // 组装一个颜色
            color = a << 24 | r << 16 | g << 8 | b;
            // 画点
            lcd_draw_point((width > 0) ? (x + x0) : (abs(width) - x - 1 + x0), (height > 0) ? (abs(height) - y - 1 + y0) : y + y0, color); // x(列) y(行) 颜色
        }
        // 跳过癞子
        i += laizi;
    }
    // 7.关闭图片
    close(bmp_fd);
}

