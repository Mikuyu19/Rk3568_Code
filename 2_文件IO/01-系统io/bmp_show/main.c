#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int lcd_fd = -1;//屏幕fd
int *plcd = NULL;//plcd指针
//lcd打开
void lcd_open()
{
    //打开屏幕
    lcd_fd = open("/dev/fb0",2);
    if(lcd_fd == -1)
    {
        perror("open lcd fail");
        return ;
    }

    //映射
    plcd = mmap(NULL,1024*600*4,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
    if(plcd == NULL)
    {
        perror("mmap fail");
        return ;
    }
}

//lcd关闭
void lcd_close()
{
    //解除隐射
    munmap(plcd,1024*600*4);

    //关闭屏幕
    close(lcd_fd);
}

/*
	画点子函数
	i:x轴
	j:y轴
*/
void lcd_draw_point(int i,int j,unsigned int color)
{
	if(i>=0&&i<1024 && j>=0&&j<600)
		*(plcd+1024*j+i) = color;
}

int main()
{
	lcd_open();
	//全部显示红色
    for(int i = 0;i<600;i++)//行
    {
        for(int j = 0;j<1024;j++)//列
        {
            lcd_draw_point(j,i,0xff0000);
        }
    }
	lcd_close();
}