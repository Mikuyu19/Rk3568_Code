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
int clientfds[50] = {0};//保存所有的客户端的fd
int client_count = 0;//客户端的数量
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



//接收数据
void* recv_data_thread(void *arg)//void*arg = new_fd  => arg = new_fd
{
    int newclientfd = *(int *)arg;
    free(arg);
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
        
        //获取指令
        char cmd[20] = {0};//命令
        char arg1[20] = {0};//参数1
        char arg2[100] = {0};//参数2
        sscanf(buf,"%s %s %s\n",cmd,arg1,arg2);

        if(strcmp(cmd,"list") == 0)//列举当前所有用户
        {
            char send_buf[1024] = {0};

            pthread_mutex_lock(&mutex);
            for(int i = 0;i<client_count;i++)
            {
                sprintf(send_buf+2*i,"%d ",clientfds[i]);   
            }
            printf("send_buf:%s\n",send_buf);
            pthread_mutex_unlock(&mutex);

            send(newclientfd,send_buf,strlen(send_buf),0);
        }
        else if(strcmp(cmd,"pm") == 0)//私聊
        {
            char send_buf[1024] = {0};

            pthread_mutex_lock(&mutex);
            for(int i = 0;i<client_count;i++)
            {
                if(clientfds[i] == atoi(arg1))
                {
                    sprintf(send_buf,"[私聊消息来自%d]:%s",newclientfd,arg2);
                    send(clientfds[i],send_buf,strlen(send_buf),0);
                }
            }
            printf("send_buf:%s\n",send_buf);
            pthread_mutex_unlock(&mutex);  
        }
        else if(strcmp(cmd,"gm") == 0)//群发
        {
            char send_buf[1024] = {0};
            sprintf(send_buf,"[群聊消息来自%d]:%s",newclientfd,arg1);

            pthread_mutex_lock(&mutex);
            for(int i = 0;i<client_count;i++)
            {
                send(clientfds[i],send_buf,strlen(send_buf),0);
            }
            printf("send_buf:%s\n",send_buf);
            pthread_mutex_unlock(&mutex);  
        }
    }

    close(newclientfd);
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

    while(1)
    {
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

        pthread_mutex_lock(&mutex);
        //将新的客户端保存到数组中
        clientfds[client_count] = newclientfd;
        client_count++;
        pthread_mutex_unlock(&mutex);

        //转门开辟一个空间用来保存accept的返回值
        int *new_fd = malloc(sizeof(int));
        *new_fd = newclientfd;

        //创建一个线程
        pthread_t thread;
        pthread_create(&thread,NULL,recv_data_thread,(void *)new_fd);        

    }

    //关闭连接
    close(sockfd);

}