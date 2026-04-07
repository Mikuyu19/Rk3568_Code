#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
//6818串口所对应的文件名
#define COM2_6818 "/dev/ttySAC1"
#define COM3_6818 "/dev/ttySAC2"
#define COM4_6818 "/dev/ttySAC3"

//RK3568串口所对应的文件名
#define COM0_3568 "/dev/ttyS0"
#define COM1_3568 "/dev/ttyS1"
#define COM3_3568 "/dev/ttyS2"
#define COM4_3568 "/dev/ttyS3"

unsigned char cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; //传感器采集命令 用来发送数据
unsigned int concen = 0;//保存获取到的烟雾浓度的值

/* 设置串口参数（初始化串口） */
void init_tty(int fd)
{    
	//1.先有保存串口属性的结构体
    //声明设置串口的结构体
    struct termios termios_new;
    //先清空该结构体
    bzero(&termios_new, sizeof(termios_new));
    
    //2.激活选项
    //cfmakeraw()设置终端属性，就是设置termios结构中的各个参数。
    cfmakeraw(&termios_new);
    //CLOCAL和CREAD分别用于本地连接和接受使能，因此，首先要通过位掩码的方式激活这两个选项。    
    termios_new.c_cflag |= CLOCAL | CREAD;
    
    //3.设置波特率  一般地，用户需将终端的输入和输出波特率设置成一样的。
    //termios_new.c_cflag=(B9600);
    cfsetispeed(&termios_new, B9600);
    cfsetospeed(&termios_new, B9600);
    
    //4.设置字符大小,通过掩码设置数据位为8位
    //与设置波特率不同，设置字符大小并没有现成可用的函数，需要用位掩码。一般首先去除数据位中的位掩码，再重新按要求设置
    termios_new.c_cflag &= ~CSIZE;/* 用数据位掩码清空数据位设置 */
    termios_new.c_cflag |= CS8; 
    
    //5.设置无奇偶校验
    termios_new.c_cflag &= ~PARENB;
    
    //6.设置一位停止位
    termios_new.c_cflag &= ~CSTOPB;
    
    //7.设置最少字符和等待时间
   	//在对接收字符和等待时间没有特别要求的情况下，可以将其设置为0，则在任何情况下read()函数立即返回
	termios_new.c_cc[VTIME] = 10;
    termios_new.c_cc[VMIN] = 1;
    
    //8.用于清空输入/输出缓冲区
    tcflush (fd, TCIFLUSH);//对接收到而未被读取的数据进行清空处理
    
    //9.完成配置后，可以使用以下函数激活串口设置
    if(tcsetattr(fd,TCSANOW,&termios_new))//配置的修改立即生效
    	printf("Setting the serial1 failed!\n");

}

void smoke()
{
	//打开串口
	int fd = open(COM1_3568,O_RDWR);
    if(fd == -1)
    {
        perror("failed open");
        return ;
    }
	//初始串口
	init_tty(fd);

    while(1)
    {
        //发送获取烟雾浓度命令
        int res = write(fd,cmd,9);
        if(res != 9)
        {
            return ;
        }
        char buf1[9] = {0};
        read(fd,buf1,9);//用来保存接收的数据
        if(buf1[0]==0xff&&buf1[1]==0x86)
        {
            concen = buf1[2]<< 8 | buf1[3];
        }
        printf("烟雾浓度为%d\n",concen);
    }   
    close(fd);

}

int main()
{
    smoke();
}