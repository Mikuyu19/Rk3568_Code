#ifndef _CIRCLE_QUEUEU_H_
#define _CIRCLE_QUEUEU_H_
#define MAXSIZE 50 //数组能存储最大元素的个数

#include "stdbool.h"

typedef int qelemtype;
typedef struct circle_queue
{
    qelemtype data[MAXSIZE];//存储数据
    int front;//保存队头元素
    int rear;//保存队尾元素
}circle_queue;

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
    实际上就是分配一个顺序栈的空间，并将栈顶指针设置为-1
*/
circle_queue *init_queue();

/*
    若栈为空,返回true,否则返回false
*/
bool queue_is_empty(circle_queue *s);

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_queue(circle_queue *s);
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
circle_queue *destroy_queue(circle_queue *s);

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool enqueue(circle_queue *s,qelemtype e);
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        qelemtype e;
        pop(s,&e)
*/
bool dequeue(circle_queue *s,qelemtype *d);

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        qelemtype e;
        get_queue_top(s,&e)
*/
int get_queue_front(circle_queue *s,qelemtype *d);

/*
    获取栈中元素的个数
*/
int get_queue_length(circle_queue *s);
#endif