#include <stdio.h>
#include <stdlib.h>
#include "linkqueue.h"

/*
    初始化操作，建立一个空队s
    创建一个空队，由s指向它
*/
linkqueue *init_queue()
{
    //为s创建空间
    linkqueue *s = malloc(sizeof(linkqueue));
    //为你的s里面的东西进行赋值
    s->front = s->rear = NULL;
    s->num = 0;
    //返回s
    return s;
}

/*
    若队为空,返回true,否则返回false
*/
bool queue_is_empty(linkqueue *s)
{
    //队不存在 || 队顶为NULL || 数据结点的个数为0
    if(s == NULL || s->front == NULL || s->num == 0)
        return true;
    
    return false;
}

/*
    将队清空
    队中没有数据就相当于清空
*/
void clean_queue(linkqueue *s)
{
    //判断队是否为空，如果为true ,直接返回
    if(queue_is_empty(s) == true)
        return ;

    //出队(头删)
    while(s->front)
    {
        //1.为你要删除的结点起个名字
        node *p = s->front;
        //2.更新s->front
        s->front = p->next;
            //发现s->front == NULL,还需要s->rear=NULL
        if(s->front == NULL)
            s->rear = NULL;
        //3.孤立要删除的结点
        p->next = NULL;
        //4.删除
        free(p);
        p = NULL;
        //5.个数--
        s->num--;
    }
    
}
/*
    若队存在，则销毁它
    释放队s占用的存储空间
*/
linkqueue *destroy_queue(linkqueue *s)
{
    if(s!=NULL)
    {
        //清空队
        clean_queue(s);
        free(s);
        s = NULL;
    }
    return s;
}

/* 
    入队
    返回值
        true 可以入队
        false 不可以入队
*/
bool enqueue(linkqueue *s,qelemtype e)
{
    //如果队不存在 返回false
    if(s == NULL)
        return false;

    //队存在  入队(头插)

    //创建结点pnew
    node *pnew = malloc(sizeof(node));
    pnew->data = e;
    pnew->next = NULL;

    //尾插
        //如果队中没有数据结点，新的节点会成为你的front和rear
    if(s->front == NULL)
    {
        s->front = s->rear = pnew;
    }
        //如果有数据结点，做尾插
    else
    {
        s->rear->next = pnew;
        s->rear = pnew;//更新rear
    }
    
    //个数+1
    s->num++;
    return true;
}
/* 
    出队
    返回值
        true 可以出队
        false 不可以出队

    调用
        qelemtype e;//开辟一个空间，用来保存出队元素
        dequeue(s,&e)
*/
bool dequeue(linkqueue *s,qelemtype *d)
{
    //判断队是否为空，如果为true ,直接返回
    if(queue_is_empty(s) == true)
        return false;

    //出队(头删)
    
    //获取数据
    *d = s->front->data;

    //1.为你要删除的结点起个名字
    node *p = s->front;
    //2.更新s->front
    s->front = s->front->next;
        //发现s->front == NULL,还需要s->rear=NULL
    if(s->front == NULL)
        s->rear = NULL;
    //3.孤立要删除的结点
    p->next = NULL;
    //4.删除
    free(p);
    p = NULL;
    //5.个数--
    s->num--;
    return true;
}

/*
    获取队顶元素的值
    返回值
        true 可以获取队顶
        false 不可以获取队顶

    调用
        qelemtype e;
        get_queue_front(s,&e)
*/
bool get_queue_front(linkqueue *s,qelemtype *d)
{
    //判断队为空
    if(queue_is_empty(s))
        return false;
    //能获取
    *d = s->front->data;
    return true;

}

/*
    获取队中元素的个数
*/
int get_queue_length(linkqueue *s)
{
    if(s==NULL)
        return 0;
    return s->num;
}



