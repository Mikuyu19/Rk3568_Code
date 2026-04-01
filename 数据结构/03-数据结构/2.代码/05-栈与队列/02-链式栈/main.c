#include <stdio.h>
#include "linkstack.h"

int main()
{
    //初始化一个栈
    linkstack *s = init_stack();

    selemtype d;
    while(1)
    {
        //输入数据 输入0结束
        scanf("%d",&d);
        if(d == 0)
            break;

        //入栈
        push(s,d);
    }

    //获取栈顶元素
    if(get_stack_top(s,&d) == true)
        printf("栈顶元素的值为%d\n",d);

    //获取栈中元素的个数
    printf("栈中元素个数为%d\n",get_stack_length(s));

    //循环出栈
    printf("出栈:");
    while(!stack_is_empty(s))//如果栈不为空就循环出栈
    {
        //出栈
        if(pop(s,&d) == false)//出栈失败 提前跳出
            break;

        //打印出栈的值
        printf("%d ",d);
    }

    //销毁栈
    s = destroy_stack(s);
}