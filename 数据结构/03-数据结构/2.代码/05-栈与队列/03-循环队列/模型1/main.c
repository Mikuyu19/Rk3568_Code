#include <stdio.h>
#include "circle_queue.h"

int main()
{
    //初始化一个队
    circle_queue *s = init_queue();

    qelemtype d;
    while(1)
    {
        //输入数据 输入0结束
        scanf("%d",&d);
        if(d == 0)
            break;

        //入队
        enqueue(s,d);
    }

    //获取队头元素
    if(get_queue_front(s,&d) == true)
        printf("队顶元素的值为%d\n",d);

    //获取队中元素的个数
    printf("队中元素个数为%d\n",get_queue_length(s));

    //循环出队
    printf("出队:");
    while(!queue_is_empty(s))//如果队不为空就循环出队
    {
        //出队
        if(dequeue(s,&d) == false)//出队失败 提前跳出
            break;

        //打印出队的值
        printf("%d ",d);
    }

    //销毁队
    s = destroy_queue(s);
}