#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include <stdlib.h>

#define LCD_WIDTH 1024
#define LCD_HEIGHT 600

extern int lcd_fd; // 屏幕fd
extern int *plcd;  // plcd指针

void lcd_open();
void lcd_close();
void lcd_draw_point(int i, int j, unsigned int color);
void bmp_display(int x0, int y0, char *bmp_path);
#endif
