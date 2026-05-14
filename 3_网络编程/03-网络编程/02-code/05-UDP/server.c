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

    //bind
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(8888);
    inet_aton("0.0.0.0",&sa.sin_addr);
    if(bind(sockfd,(struct sockaddr*)&sa,sizeof(sa)) == -1)
    {
        perror("bind failed");
        return -1;
    }

    while(1)
    {
        //recvfrom
        struct sockaddr_in addr;
        socklen_t addrlen = sizeof(addr);
        char buf[100] = {0};
        recvfrom(sockfd,buf,sizeof(buf)-1,0,(struct sockaddr*)&addr,&addrlen);
        printf("%s[%d]:%s",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),buf);

        //senfto 回复
        sendto(sockfd,"ok",strlen("ok"),0,(struct sockaddr*)&addr,sizeof(addr));
    }
    //close
    close(sockfd);
}