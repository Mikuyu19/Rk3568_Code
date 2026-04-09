#include <stdio.h>
#include <unistd.h>

int main()
{
    char *p = "hello";
    int i = 0;
    while(p[i]!='\0')
    {
        putchar(p[i]);//打一个字符
        fflush(stdout);//把缓冲区的数据同步到终端
        sleep(1);//1s
        i++;
    }
    printf("\n");
}