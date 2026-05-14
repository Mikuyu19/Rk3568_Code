#include <stdio.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

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

    int clientfds[50] = {0};//保存所有的客户端的fd
    int client_count = 0;//客户端的数量
    int maxfd = sockfd;//初始化最大的文件描述符    
    while(1)
    {
        //1.定义一个集合
        fd_set readfds;//可读文件描述符集合
        //2.清空集合
        FD_ZERO(&readfds);
        //3.把你需要要监听的fd加入到可读集合
        FD_SET(sockfd, &readfds);//添加服务器的fd
        for(int i = 0;i<client_count;i++)
        {
            FD_SET(clientfds[i], &readfds);//添加全部客户端的fd
        }
        //4.设置超时时间 5s
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        //5.select
        int ret = select(maxfd+1,&readfds,NULL,NULL,&tv);
        //6.处理结果
        if(ret > 0)//检查具体的fd
        {
            if(FD_ISSET(sockfd,&readfds))//sockfd是socket的返回值 sockfd就绪
            {
                //代表有客户端想你发起连接
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
                clientfds[client_count] = newclientfd;
                client_count++;//客户端的数量+1
                if(maxfd < newclientfd)
                {
                    //更新maxfd
                    maxfd = newclientfd;
                }
            }
            for(int i = 0;i<client_count;i++)
            {
                if(FD_ISSET(clientfds[i],&readfds))//判断clientfds[i]是否就绪
                {
                    //有客户端给你发送数据 read/recv/recvfrom
                    //read/recv/recvfrom 返回值0 代表有客户端退出 把clientfds[i]在clientfds数组中删除
                    char buf[100] = {0};
                    int ret = recv(clientfds[i],buf,sizeof(buf)-1,0);
                    if(ret == 0)
                    {
                        printf("对方已经断开连接\n");
                        close(clientfds[i]);
                        //把clientfds[i]在clientfds数组中删除
                        for(int j = i;j<client_count-1;j++)
                        {
                            clientfds[j] = clientfds[j+1];
                        }
                        client_count--;
                    }
                    else if(ret > 0)
                    {
                        printf("read:%s",buf);
                        char cmd[10] = {0};//命令
                        char arg1[100] = {0};//参数1
                        char arg2[100] = {0};//参数2
                        sscanf(buf,"%s %s %s\n",cmd,arg1,arg2);
                        if(strcmp(cmd,"list") == 0)
                        {
                            char send_buf[1024] = {0};//保存要发送的数据
                            sprintf(send_buf,"list:");
                            for(int i = 0;i<client_count;i++)
                            {
                                sprintf(send_buf+strlen(send_buf),"%d ",clientfds[i]);
                            }
                            printf("send_buf:%s\n",send_buf);
                            send(clientfds[i],send_buf,strlen(send_buf),0);
                        }
                        else if(strcmp(cmd,"pm") == 0)
                        {
                            char send_buf[1024] = {0};//保存要发送的数据
                            for(int j = 0;j<client_count;j++)
                            {
                                if(clientfds[j] == atoi(arg1))//找到了
                                {
                                    sprintf(send_buf,"pm:[私聊消息来自%d]:%s",clientfds[i],arg2);
                                    send(clientfds[j],send_buf,strlen(send_buf),0);
                                }
                                
                            }
                            
                        }
                        else if(strcmp(cmd,"gm") == 0)
                        {
                            char send_buf[1024] = {0};//保存要发送的数据
                            for(int j = 0;j<client_count;j++)
                            {

                                sprintf(send_buf,"gm:[群发消息来自%d]:%s",clientfds[i],arg1);
                                send(clientfds[j],send_buf,strlen(send_buf),0);
 
                            }
                            
                        }
                    }

                }
            }
        }
        else if(ret == 0)
        {
            printf("超时\n");
        }  
        else if(ret < 0)
        {
            perror("select failed");
            break;
        }

    }

    //关闭连接
    close(sockfd);

}