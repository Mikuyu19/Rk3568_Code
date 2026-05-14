#include <sys/types.h>          /* See NOTES */  
#include <sys/socket.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int init_client_socket(char *ip,char *port)
{
    //创建套接字
    int client_sock = socket(AF_INET,SOCK_STREAM,0);
    if(client_sock == -1)
    {
        perror("socket failed");
        return -1;
    }
    //connect客户端连接服务器
    struct sockaddr_in sa;
	sa.sin_family = AF_INET;//ipv4
	sa.sin_port=htons(atoi(port));//指定端口号  把端口号主机字节h序转化为网络字节序n
	inet_aton(ip,&sa.sin_addr);//指定ip 将点分十进制ip地址转化为二进制网络字节序ip
	printf("连接服务器\n");
    if(connect(client_sock,(struct sockaddr *)&sa,(socklen_t)sizeof(sa)))
    {
        perror("connect");
        close(client_sock);
        return -1;
    }
    printf("成功连接服务器\n");

    return client_sock;
}

//./a.out 172.6.1.209 8888 1.txt
int main(int argc,char**argv)
{
    if(argc<3)
    {
        printf("input:%s <ip> <port>\n",argv[0]);
        return -1;
    }
    int client_sock = init_client_socket(argv[1],argv[2]);
    
    char *buff = "hello,abc";
    while(1)
    {	
        //求buf的字串的长度
        int length = strlen(buff);
        //pbuff开的空间为length+4
        char *pbuff = malloc(sizeof(char)*(length+4));
        memcpy(pbuff,&length,4);
        memcpy(pbuff+4,buff,length);
        int wbytes = send(client_sock,pbuff,length+4,0);
        if(wbytes < 0)
        {
            perror("send ");
            exit(EXIT_FAILURE);
        }
        usleep(1000);
        free(pbuff);
    }

    //关闭客户端套接字
    close(client_sock);
}