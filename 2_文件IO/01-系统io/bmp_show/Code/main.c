#include "lcd.h"
#include "touch.h"
int main()
{
    //用字符数组保存所有的图片名字
    char *bmp[] = {"1.bmp","2.bmp","3.bmp"};
    int i = 0;//i保存要显示图片的下标

    //打开屏幕
	lcd_open();


    while(1)
    {
        //显示图片
        bmp_display(0,0,bmp[i]);
        printf("display %s\n",bmp[i]);

        //点击屏幕
        get_xy();

        if(read_x>=0 && read_x < 500)//左边
        {
            printf("left\n");
            i--;
            if(i == -1)
                i = 2;
        }
        else if(read_x>=550 && read_x < 1024)//右边
        {
            printf("right\n");
            i++;
            if(i == 3)
                i = 0;   
        }
    }

    //关闭屏幕
	lcd_close();
}