#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
//6818串口所对应的文件名
#define COM2 "/dev/ttySAC1"
#define COM3 "/dev/ttySAC2"
#define COM4 "/dev/ttySAC3"


int data[5] = {0};

//定义命令--->设置模块的工作方式
unsigned char cmd1[3]={0xa5,0x81,0x26};//设置连续光照模式
unsigned char cmd2[3]={0xa5,0x82,0x27};//设置温湿度气压海拔模式

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

//GY39模块
void GY39()
{
    //1.打开串口
    int ttyfd = open(COM2,O_RDWR);
    if(-1 == ttyfd)
    {
        perror("open ttyfd fail");
        return ;
    }
    //2.初始化串口
	init_tty(ttyfd);
	
    unsigned char rbuf1[9];
    unsigned char rbuf2[15];
    int Lux,T,P,HUM,H;
    int ret = write(ttyfd,cmd1,3);//获取光照强度
    if(ret!=3)
    {
        sleep(1);//等待完全写入 
    }
    
    read(ttyfd,rbuf1,9);
    if(rbuf1[0]==0x5A&&rbuf1[1]==0x5A&&rbuf1[2]==0x15)//判断帧是否正常
    {
        Lux=(rbuf1[4]<<24)|(rbuf1[5]<<16)|(rbuf1[6]<<8)|rbuf1[7];
        data[0] = Lux=Lux/100;//最后一位不要 不显示小数
        printf("light = %d\n",Lux);
    }
    //获取气压海拔等....
    //S1 切换模式
    ret = write(ttyfd,cmd2,3);
    if(ret!=3)
    {
        sleep(1);//等待完全写入 
    }

    read(ttyfd,rbuf2,15);
    if(rbuf2[0]==0x5A&&rbuf2[1]==0x5A&&rbuf2[2]==0x45)//判断帧是否正常
    {
		//温度
        T=(rbuf2[4]<<8)|rbuf2[5];
        data[1] = T=T/100;
        printf("T=%d\n",T);
        //气压湿度海拔自己完成 <---练习
		//气压
		P = (rbuf2[6]<<24)|(rbuf2[7]<<16)|(rbuf2[8]<<8)|rbuf2[9];
		data[2] = P = P/100;
		printf("P=%d\n",P);
		//湿度
		HUM=(rbuf2[10]<<8)|rbuf2[11];
        data[3] = HUM=HUM/100;
		printf("HUM=%d\n",HUM);
		//海拔
		H=(rbuf2[12]<<8)|rbuf2[13];
        data[4] = H=H;
		printf("H=%d\n",H);
    }
    close(ttyfd);
    return ;
}