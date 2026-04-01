#include <stdio.h>

#define MAXSIZE 100
typedef struct 
{
    int data;
    int next;//游标  为0表示无指向
}static_list;

void init_list(static_list s[])
{
    for(int i = 0;i<MAXSIZE-1;i++)
    {
        s[i].next = i+1;
    }

    s[MAXSIZE-1].next = 0;//静态链表为空

    //最后一个结点的next如果为0 单表链表为空
    //最后一个结点的next如果为其它 第一个结点所在的下标
}

int malloc_space_index(static_list s[])
{
    int i = s[0].next;
    s[0].next=s[i].next;

    return i;
}

void tail_insert(static_list s[],int d)
{
    int new = malloc_space_index(s);
    s[new].data=d;
    

    if(s[MAXSIZE-1].next == 0)//空链表
    {
        s[MAXSIZE-1].next = new;
    }
    else
    {
        //找最后一个
        int p = s[MAXSIZE-1].next;//遍历指针
        int pre = 0;
        while(p)
        {
            pre=p;//p成为pre
            p=s[p].next;//p=p->next i++
        }
        //尾插
        s[pre].next=new;
    }
}
void print_static_list(static_list s[])
{
    int p = s[MAXSIZE-1].next;
    while(p)
    {
        printf("%d ",s[p].data);
        p=s[p].next;
    }
    printf("\n");
}

//回收删除结点的下标
void free_space_index(static_list s[],int p)
{
    s[p].next = s[0].next;//回收结点的下一个是备用链表第一个结点
    s[0].next = p;//把回收的结点作为备用链表第一个结点
}

void delete_x_index(static_list s[],int x)
{
    //找值为x的结点
    int p = s[MAXSIZE-1].next;
    int pre = 0;
    while(p)
    {
        if(s[p].data == x)//找到了
            break;
        pre=p;
        p=s[p].next;//p=p->next i++
    }

    if(p!=0)
    {
        //头删
        if(p == s[MAXSIZE-1].next)
        {
            s[MAXSIZE-1].next =s[p].next;
            free_space_index(s,p);
        }
        else//中间删 尾删
        {
            //让pre的下一个为p的下一个
            s[pre].next = s[p].next;
            free_space_index(s,p);
        }
    }
}

int main()
{
    static_list s[MAXSIZE];
    init_list(s);

    int d;
    while(1)
    {
        scanf("%d",&d);
        if(d == 0)
            break;
        tail_insert(s,d);
    }

    print_static_list(s);

}