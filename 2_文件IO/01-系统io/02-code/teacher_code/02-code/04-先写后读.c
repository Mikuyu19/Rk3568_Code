#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int main()
{
    //打开
    int fd = open("./2.txt",2);
    if(fd == -1)//失败
    {
        perror("open failed");
        return 0;
    }

    printf("ok\n");

    //写入
    int ret = write(fd,"asdhasdgsd",strlen("asdhasdgsd"));
    printf("成功写入%d个字节\n",ret);

    //移动光标到文件开头
    lseek(fd,0,SEEK_SET);

    //读取
    char buf[100] = {0};
    ret = read(fd,buf,ret);
    printf("成功读取到%d个字节:%s\n",ret,buf);



}