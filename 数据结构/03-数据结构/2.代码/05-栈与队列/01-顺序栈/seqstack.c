#include <stdio.h>
#include <stdlib.h>
#include "seqstack.h"

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
    实际上就是分配一个顺序栈的空间，并将栈顶指针设置为-1
*/
seqstack *init_stack()
{
    seqstack *s = (seqstack *)malloc(sizeof(seqstack));//分配一个顺序栈的空间，首地址存放在s
    s->top = -1;//栈顶指针置为-1,表明没有存放数据
    return s;//返回s中存储的地址
}

/*
    若栈为空,返回true,否则返回false
*/
bool stack_is_empty(seqstack *s)
{
    //栈不存在 || 一个数据元素都没有 
    if(s == NULL || s->top == -1)
        return true;
    return false;
}

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_stack(seqstack *s)
{
    //栈如果不存在 直接返回
    if(s == NULL)
        return ;

    //栈存在，把栈变回初始化的样子即可
    s->top = -1;//没有数据
}
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
seqstack *destroy_stack(seqstack *s)
{
    //把申请的空间free即可
    if(s!=NULL)
    {
        free(s);
        s = NULL;
    }

    return s;
}

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool push(seqstack *s,selemtype e)
{
    //栈不存在 或 栈满 不能入栈
    if(s==NULL || s->top ==MAXSIZE-1)
        return false;

    //入栈操作
    s->top++;//s->top = s->top+1;  //先让top+1
    s->data[s->top] = e;//再把数据放到top指向的位置
    return true;
}
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        selemtype e;
        pop(s,&e)
*/
bool pop(seqstack *s,selemtype *d)//d = &e  ==> *d = *&e = e
{
    //调用判断栈是否为空的函数,若为true,不能出栈
    if(stack_is_empty(s) == true)
        return false;
    //出栈
    *d = s->data[s->top];
    s->top--;
    return true;
}

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        selemtype e;
        get_stack_top(s,&e)
*/
int get_stack_top(seqstack *s,selemtype *d)
{
    //调用判断栈是否为空的函数,若为true,不能出栈
    if(stack_is_empty(s) == true)
        return false;
    
    *d = s->data[s->top];
    return true;
}

/*
    获取栈中元素的个数
*/
int get_stack_length(seqstack *s)
{
    if(s == NULL)
        return 0;
    return s->top+1;
}

