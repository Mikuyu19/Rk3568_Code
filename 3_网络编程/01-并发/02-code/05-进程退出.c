#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
    printf("abc");
    #if 0
    _exit(0);//等价于 _Exit(0)  不会做清理工作，会把缓冲区的内容直接丢弃
    #else
    exit(0);//等价于 return 0;  会做清理工作，会把缓冲区的内容输出到终端
    #endif

}