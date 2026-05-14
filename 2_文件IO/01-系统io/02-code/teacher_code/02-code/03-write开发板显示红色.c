#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    //打开屏幕
    int lcd_fd = open("/dev/fb0",2);
    if(lcd_fd == -1)
    {
        perror("open lcd fail");
        return 0;
    }

    //写入
    unsigned int color = 0xff0000;//红色
    for(int i = 0;i<600;i++)//行
    {
        for(int j = 0;j<1024;j++)//列
        {
             write(lcd_fd,&color,4);
        }
    }
   
    //关闭屏幕
    close(lcd_fd);
}
