#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//接收数据
void* recv_data_thread(void *arg)//void*arg = new_fd  => arg = new_fd
{
    int sockfd = *(int *)arg;
    //接收数据
    while(1)
    {
        char buf[100] = {0};
        int ret = recv(sockfd,buf,sizeof(buf)-1,0);
        if(ret == 0)
        {
            printf("对方已经断开连接\n");
            break;
        }
        printf("read:%s\n",buf);
        if(strcmp(buf,"q\n") == 0)
            break;
    }

    close(sockfd);
}
int main()
{
    //创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket failed");
        return -1;
    }

    //发起连接
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;//ipv4
	sa.sin_port = htons(8888);//指定端口号，把端口号主机字节序转成网络字节序
	inet_aton("172.50.1.11", &sa.sin_addr);//指定ip 将点分十进制的ip地址转成二进制网络ip
	if(connect(sockfd, (struct sockaddr *)&sa,sizeof(sa)) == -1)
	{
		perror("bind failed");
		return -1;
	}   
    //创建一个线程
    pthread_t thread;
    pthread_create(&thread,NULL,recv_data_thread,(void *)&sockfd);            
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