#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    #if 0
    int ret = truncate("3.txt", 500);    
    if(ret == -1)
    {
        perror("truncate fail");
        return -1;
    }
    #endif
    #if 1
    int fd = open("3.txt",2);
    int ret = ftruncate(fd, 500);    
    if(ret == -1)
    {
        perror("truncate fail");
        return -1;
    }
    #endif
}