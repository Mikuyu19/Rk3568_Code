// 练习：利用fgetc和fputc实现从键盘输入字符，再显示到标准输出中去，直到遇到‘#’才结束
#include <stdio.h>
#include <stdlib.h>

int main()
{
    while (1)
    {
        char ch = fgetc(stdin); // 从键盘输入一个字符
        if (ch == '#')
        {
            break; // 遇到#结束循环
        }
        fputc(ch, stdout); // 将输入的字符显示到标准输出
    }
    return 0;
}

