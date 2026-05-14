#include <stdio.h>
#include <stdlib.h>
#include "circle_queue.h"

/*
    初始化操作，建立一个空队s
    创建一个空队，由s指向它
    实际上就是分配一个顺序队的空间，并将队顶指针设置为-1
*/
circle_queue *init_queue()
{
    circle_queue *q = (circle_queue *)malloc(sizeof(circle_queue));//分配一个顺序队的空间，首地址存放在s
    q->front = q->rear = 0;
    return q;//返回q中存储的地址
}

/*
    若队为空,返回true,否则返回false
*/
bool queue_is_empty(circle_queue *q)
{
    //队不存在 || 一个数据元素都没有 
    if(q == NULL || q->front == q->rear)
        return true;
    return false;
}

/*
    将队清空
    队中没有数据就相当于清空
*/
void clean_queue(circle_queue *q)
{
    //队如果不存在 直接返回
    if(q == NULL)
        return ;

    //队存在，把队变回初始化的样子即可
    q->front = q->rear = 0;//没有数据
}
/*
    若队存在，则销毁它
    释放队s占用的存储空间
*/
circle_queue *destroy_queue(circle_queue *q)
{
    //把申请的空间free即可
    if(q!=NULL)
    {
        free(q);
        q = NULL;
    }

    return q;
}

/* 
    入队
    返回值
        true 可以入队
        false 不可以入队
*/
bool enqueue(circle_queue *q,qelemtype e)
{
    //队不存在 或 队满 不能入队
    if(q==NULL || (q->rear+1)%MAXSIZE == q->front)
        return false;

    //入队操作
    q->rear = (q->rear+1)%MAXSIZE;//先让rear+1
    q->data[q->rear] = e;//再把数据放到rear指向的位置
    return true;
}
/* 
    出队
    返回值
        true 可以出队
        false 不可以出队

    调用
        qelemtype e;
        pop(s,&e)
*/
bool dequeue(circle_queue *q,qelemtype *d)//d = &e  ==> *d = *&e = e
{
    //调用判断队是否为空的函数,若为true,不能出队
    if(queue_is_empty(q) == true)
        return false;
    //出队
    q->front = (q->front+1)%MAXSIZE;//front+1
    *d = q->data[q->front];

    return true;
}

/*
    获取队顶元素的值
    返回值
        true 可以获取队顶
        false 不可以获取队顶

    调用
        qelemtype e;
        get_queue_top(s,&e)
*/
int get_queue_front(circle_queue *q,qelemtype *d)
{
    //调用判断队是否为空的函数,若为true,不能出队
    if(queue_is_empty(q) == true)
        return false;
    
    *d = q->data[(q->front+1)%MAXSIZE];
    return true;
}

/*
    获取队中元素的个数
*/
int get_queue_length(circle_queue *q)
{
    if(q == NULL)
        return 0;
    return (q->rear-q->front+MAXSIZE)%MAXSIZE;
}

