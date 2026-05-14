#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#define unix_path "/home/china/unix2612.socket"
int main()
{
    //创建套接字
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket failed");
        return -1;
    }


    //发起连接
	struct sockaddr_un sa;
	sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path,unix_path);
    if(connect(sockfd, (struct sockaddr *)&sa,sizeof(sa)) == -1)
	{
		perror("bind failed");
		return -1;
	}   
    
    //发送数据
    while(1)
    {
        char buf[100] = {0};//用来保存从键盘获取到的字符串
        fgets(buf,sizeof(buf)-1,stdin);//从键盘输入
        send(sockfd,buf,strlen(buf),0);
        if(strcmp(buf,"q\n") == 0)
            break;
    }

    //关闭连接
    close(sockfd);
    
}