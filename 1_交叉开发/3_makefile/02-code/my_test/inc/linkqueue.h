#ifndef _LINKQUEUE_H_
#define _LINKQUEUE_H_

#include "stdbool.h"
#include "bst.h"
//数据类型的定义
typedef binode* qelemtype;
//队列中的数据结点类型
typedef struct node
{
    qelemtype data;//数据
    struct node *next;//存放下一个结点的地址
}node;
//链式队列类型
typedef struct linkqueue
{
    node *front;//队头指针
    node *rear;//队尾指针
    int num;//数据结点的个数
}linkqueue;

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
    实际上就是分配一个顺序栈的空间，并将栈顶指针设置为-1
*/
linkqueue *init_queue();

/*
    若栈为空,返回true,否则返回false
*/
bool queue_is_empty(linkqueue *s);

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_queue(linkqueue *s);
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
linkqueue *destroy_queue(linkqueue *s);

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool enqueue(linkqueue *s,qelemtype e);
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        qelemtype e;
        pop(s,&e)
*/
bool dequeue(linkqueue *s,qelemtype *d);

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        qelemtype e;
        get_queue_top(s,&e)
*/
bool get_queue_front(linkqueue *s,qelemtype *d);

/*
    获取栈中元素的个数
*/
int get_queue_length(linkqueue *s);
#endif