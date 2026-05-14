#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int recv_file(int newclientfd)
{
	//切换工作路径，不能与传输的文件在同一路径下
	chdir("../");
	//接收文件名的长度大小
    int file_name_length = 0;
    recv(newclientfd,&file_name_length,sizeof(file_name_length),0);
    printf("file_name_length = %d\n",file_name_length);

	//接收文件名
    char file_name[100] = {0};
    recv(newclientfd,&file_name,file_name_length,0);
    printf("file_name = %s\n",file_name);	

	//接收文件大小
    int file_size = 0;
    recv(newclientfd,&file_size,sizeof(file_size),0);
    printf("file_size = %d\n",file_size);	
	
	//接收文件内容
	int fd = open(file_name,O_CREAT|O_WRONLY|O_TRUNC,0777);//打开文件

    int total_recv_size = 0;//记录已经接收了多少个字节
	while(1)//循环到什么时候结束，接收的总字节数 等于 接收的文件大小
	{
        char buf[100] = {0};
		int ret = recv(newclientfd,buf,sizeof(buf)-1,0);//读取来自客户端的数据
        if(ret == 0)
            break;
        write(fd,buf,ret);//写入到文件
        total_recv_size+=ret;//累加
        if(file_size <= total_recv_size)//接收完毕
        {
            break;
        }
	}
	//关闭文件
    close(fd);
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

    //绑定自己的ip和端口号
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;//ipv4
	sa.sin_port = htons(8888);//指定端口号，把端口号主机字节序转成网络字节序
	inet_aton("0.0.0.0", &sa.sin_addr);//指定ip 将点分十进制的ip地址转成二进制网络ip
	/*
		监听发送给172.50.1.11的数据包
		还可以写127.0.0.1 -> 同一台主机通信
		还可写0.0.0.0  -> 服务器绑定ip更推荐用这个
		表示监听服务器主机上所有的ipv4的地址，也就是说
		无论是从哪个ip地址发送和过来，只要是目的端口和监听的端口相匹配的
		服务器都可以处理数据包
	*/
	if(bind(sockfd, (struct sockaddr *)&sa,sizeof(sa)) == -1)
	{
		perror("bind failed");
		return -1;
	}

    //监听
    if(listen(sockfd,5) == -1)
    {
        perror("listen failed");
        return -1;
    }

    //接收客户端连接
    #if 0
    //不想知道对方的网络地址
    int newclientfd = accept(sockfd, NULL, NULL);
    if(newclientfd == -1)
    {
        perror("accept failed");
        return -1;
    }
    printf("客户端已成功连接\n");
    #else
    //想知道对方的网络地址
	struct sockaddr_in client_addr;//保存对方的网络地址
	socklen_t client_addrlen = sizeof(struct sockaddr_in);//保存对方网络地址大小
	int newclientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
    if(newclientfd == -1)
    {
        perror("accept failed");
        return -1;
    }	
    //打印对象的ip地址和端口号
	printf("%s[%d]已成功连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));    
    #endif

    //接收文件
    recv_file(newclientfd);

    //关闭连接
    close(newclientfd);
    close(sockfd);

}