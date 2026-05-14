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

int main()
{
    //socket
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd == -1)
    {
        perror("socket failed");
        return -1;
    }    

    //准备发送的网络地址
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8888);
    inet_aton("172.50.1.11",&sa.sin_addr);


    while(1)
    {
        //sendto
        char buf[100] = {0};
        fgets(buf,sizeof(buf)-1,stdin);//从键盘获取字符串
        sendto(sockfd,buf,strlen(buf),0,(struct sockaddr*)&sa,sizeof(sa));

        //recvform 回复
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        char buf2[100] = {0};
        recvfrom(sockfd,buf2,sizeof(buf2)-1,0,(struct sockaddr*)&addr,&addrlen);
        printf("%s[%d]:%s",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),buf2);
        
        if(strcmp(buf,"q\n") == 0)
            break;
    }

    //close
    close(sockfd);
}