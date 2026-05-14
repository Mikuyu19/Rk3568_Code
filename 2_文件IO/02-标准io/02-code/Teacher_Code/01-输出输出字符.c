#include <stdio.h>

int main()
{
#if 0
    while(1)
    {
        //从键盘获取字符
        char c = fgetc(stdin);
        //判断获取的字符是否为# 是则跳出循环 否执行后面代码
        if(c == '#')
            break;

        //打印输出
        fputc(c, stdout);
    }
#else
    char c;
    while((c = fgetc(stdin))!='#')
    {
        //打印输出
        fputc(c, stdout);
    }
#endif
}