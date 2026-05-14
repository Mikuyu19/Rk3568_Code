#ifndef _SEQSTACK_H_
#define _SEQSTACK_H_
#define MAXSIZE 50 //数组能存储最大元素的个数

#include "stdbool.h"

typedef int selemtype;
typedef struct seqstack
{
    selemtype data[MAXSIZE];//存储数据
    int top;//保存栈顶元素
}seqstack;

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
    实际上就是分配一个顺序栈的空间，并将栈顶指针设置为-1
*/
seqstack *init_stack();

/*
    若栈为空,返回true,否则返回false
*/
bool stack_is_empty(seqstack *s);

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_stack(seqstack *s);
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
seqstack *destroy_stack(seqstack *s);

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool push(seqstack *s,selemtype e);
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        selemtype e;
        pop(s,&e)
*/
bool pop(seqstack *s,selemtype *d);

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        selemtype e;
        get_stack_top(s,&e)
*/
int get_stack_top(seqstack *s,selemtype *d);

/*
    获取栈中元素的个数
*/
int get_stack_length(seqstack *s);
#endif