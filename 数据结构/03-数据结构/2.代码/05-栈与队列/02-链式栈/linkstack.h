#ifndef _LINKSTACK_H_
#define _LINKSTACK_H_

#include "stdbool.h"

//数据类型的定义
typedef int selemtype;
//栈中的数据结点类型
typedef struct node
{
    selemtype data;//数据
    struct node *next;//存放下一个结点的地址
}node;
//链式栈类型
typedef struct linkstack
{
    node *top;//栈顶指针
    node *bottom;//栈底指针
    int num;//数据结点的个数
}linkstack;

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
    实际上就是分配一个顺序栈的空间，并将栈顶指针设置为-1
*/
linkstack *init_stack();

/*
    若栈为空,返回true,否则返回false
*/
bool stack_is_empty(linkstack *s);

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_stack(linkstack *s);
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
linkstack *destroy_stack(linkstack *s);

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool push(linkstack *s,selemtype e);
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        selemtype e;
        pop(s,&e)
*/
bool pop(linkstack *s,selemtype *d);

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        selemtype e;
        get_stack_top(s,&e)
*/
bool get_stack_top(linkstack *s,selemtype *d);

/*
    获取栈中元素的个数
*/
int get_stack_length(linkstack *s);
#endif