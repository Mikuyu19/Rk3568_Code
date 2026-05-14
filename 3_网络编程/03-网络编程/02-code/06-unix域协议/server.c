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
	//删除
	unlink(unix_path);
    
    //创建套接字
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd == -1)
    {
        perror("socket failed");
        return -1;
    }


    //绑定文件系统路径
	struct sockaddr_un sa;
	sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path,unix_path);
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
    int newclientfd = accept(sockfd, NULL, NULL);
    if(newclientfd == -1)
    {
        perror("accept failed");
        return -1;
    }
    printf("客户端已成功连接\n");

    //接收数据
    while(1)
    {
        char buf[100] = {0};
        int ret = recv(newclientfd,buf,sizeof(buf)-1,0);
        if(ret == 0)
        {
            printf("对方已经断开连接\n");
            break;
        }
        printf("read:%s",buf);
        if(strcmp(buf,"q\n") == 0)
            break;
    }

    //关闭连接
    close(newclientfd);
    close(sockfd);

}