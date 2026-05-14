#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int send_file(int sockfd,char *file_name)
{
	//发送文件名的长度大小
    int file_name_length = strlen(file_name);//求文件名的长度
    send(sockfd,&file_name_length,sizeof(file_name_length),0);

	//发送文件名
    send(sockfd,file_name,file_name_length,0);
	
	//发送文件大小
    struct stat st;//保存文件的属性
	stat(file_name,&st);//获取属性保存到st中
    int file_size = st.st_size;
    send(sockfd,&file_size,sizeof(file_size),0);

	//发送文件内容
	int fd = open(file_name,0);//打开文件
	while(1)//循环到什么时候结束，读到文件末尾结束
	{
        char buf[100] = {0};
		int ret = read(fd,buf,sizeof(buf)-1);//读取文件内容
        if(ret == 0)//读取文件结束
            break;
		send(sockfd,buf,ret,0);//再把读到的内容发送给服务器
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
    
    //发送文件
    send_file(sockfd,"1.txt");
    //关闭连接
    close(sockfd);
    
}