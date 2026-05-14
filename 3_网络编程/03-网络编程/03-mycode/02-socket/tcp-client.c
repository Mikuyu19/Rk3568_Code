#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    char buffer[1024];
    while (1)
    {
        printf("send: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;
        // if (write(sockfd, buffer, strlen(buffer)) == -1)
        // {
        //     perror("write");
        //     break;
        // }
        if (send(sockfd, buffer, strlen(buffer), 0) == -1)
        {
            perror("send");
            break;
        }
        if (strcmp(buffer, "q") == 0)
        {
            printf("quitting...\n");
            break;
        }
    }
    close(sockfd);
    return 0;
}
