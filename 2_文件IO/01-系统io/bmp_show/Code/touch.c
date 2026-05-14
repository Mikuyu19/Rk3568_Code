#include "touch.h"

int read_x,read_y;//保存读取到的坐标的值

void get_xy()
{
    int x = -1,y = -1;//保存读取到的坐标的值
    //打开触摸屏幕
    int touch_fd = open("/dev/input/event6",2);
    if(touch_fd == -1)
    {
        perror("open touch failed");
        return ;
    }

    //循环读取触摸屏的数据
    struct input_event ts;//保存读取到的数据
    while(1)
    {
        //读取一次触摸屏的数据
        read(touch_fd,&ts,sizeof(ts));

        if(ts.type == EV_ABS)//触摸屏
        {
            if(ts.code == ABS_X)//x
            {
                x = ts.value;
            }
            else if(ts.code == ABS_Y)//y
            {
                y = ts.value;
            }
        }

        //获取一次坐标 跳出循环
        if(x>=0 && y>=0)
        {
            read_x = x;
            read_y = y;
            break;
        }
    }

    //关闭屏幕
    close(touch_fd);
}
