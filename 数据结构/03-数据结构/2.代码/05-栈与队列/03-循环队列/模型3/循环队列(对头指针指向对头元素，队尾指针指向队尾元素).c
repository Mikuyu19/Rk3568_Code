#include <stdio.h>
#include <stdlib.h>
#define MAXSIZE 5

typedef int qelemtype;
typedef struct circle_queue
{
    int data[MAXSIZE];//数据域
    int front;
    int rear;
    int flag;//保存最后一次的操作时出队还是入队 出队0 入队1
}circle_queue;

circle_queue *init_queue()
{
    circle_queue*q = malloc(sizeof(circle_queue));
    q->front = 1;
    q->rear = 0;
    q->flag = 0;//初始化设置成出队
    return q;
}

void clear_queue(circle_queue*q)
{
    if(q == NULL)//队列不存在
        return ;
    q->front = 1;
    q->rear = 0;
    q->flag = 0;//初始化设置成出队
}

circle_queue *destroy_queue(circle_queue*q)
{
    if(q!=NULL)
    {
        free(q);
        q = NULL;
        return q;
    }
}

int queue_is_empty(circle_queue*q)
{
    if(q == NULL)
        return 1;//队列不存在

    if((q->rear+1)%MAXSIZE == q->front && q->flag == 0)//队尾元素的下一个是对头元素&&最后操作出队
        return 1;//队空

    return 0;
}

//入队
int enqueue(circle_queue*q,qelemtype x)
{
    if(q == NULL)
        return 0;//入队失败

    if((q->rear+1)%MAXSIZE == q->front && q->flag == 1)//队尾元素的下一个是对头元素&&最后操作入队
        return 0;//队满

    q->rear = (q->rear+1)%MAXSIZE;//队尾指针先走
    q->data[q->rear] = x;//放数据
    q->flag = 1;//记录最后操作是入队

    return 1;//入队成功
}

//出队操作
int dequeue(circle_queue *q,qelemtype *x)
{
    if(queue_is_empty(q))
        return 0;//队空 出队失败
    
    *x = q->data[q->front];//先获取对头元素
    q->front = (q->front+1)%MAXSIZE;//对头指针再走
    q->flag = 0;//记录最后操作是出队

    return 0;//入队成功
}

int queue_length(circle_queue *q)
{
    if(queue_is_empty(q))
    {
        return 0;
    }
    
    if((q->rear+1)%MAXSIZE == q->front && q->flag == 1)//队尾元素的下一个是对头元素&&最后操作入队，队列为满
        return MAXSIZE;
    else
        return (q->rear-q->front+1+MAXSIZE)%MAXSIZE;
}


int main()
{
    //初始化队列
    circle_queue *q = init_queue();
    //设置MAXSIZE为5 入6个元素
    for(int i = 1;i<=6;i++)
    {
        if(enqueue(q,i) == 0)//入队失败
        {
            printf("%d入队失败\t",i);
        }
        else//入队成功
        {
            printf("%d入队成功\t",i);
        }
        printf("当前队列元素个数为%d\n",queue_length(q));
    }

    qelemtype x;
    //循环出队
    while(!queue_is_empty(q))
    {
        dequeue(q,&x);
        printf("%d出队成功\t",x);
        printf("当前队列元素个数为%d\n",queue_length(q));
    }

    q = destroy_queue(q);
    
}