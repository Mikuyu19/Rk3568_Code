#include <stdio.h>
#include "circle_list.h"
#include <stdlib.h>

//初始化双向循环链表  没有数据结点 只有一个头结点
dnode *init_circle_list()
{
	dnode *head = malloc(sizeof(dnode));
	head->data = 0;
	head->next = head->prev = head;
}

//创建一个新的结点
dnode *malloc_new_dnode(delemtype d)
{
	dnode *pnew = malloc(sizeof(dnode));
	pnew->data = d;
	pnew->next=pnew->prev= NULL;
	return pnew;
}
/*
	尾插
*/
void tail_insert_circle_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //找尾巴
    dnode *tail = head->prev;
    //tail就是尾巴

    //尾插
    pnew->next = head;
    pnew->prev = tail;
    tail->next = pnew;
    head->prev = pnew;
}
/*
	头插
*/
void head_insert_circle_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //头插
    pnew->next = head->next;
    pnew->prev = head;
    head->next->prev = pnew;
    head->next = pnew;
}
/*
	有序插入
*/
void sort_asc_insert_circle_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //找第一个比pnew大的结点
    dnode *p = head->next;
    dnode *pre = head;
    while(p!=head)
    {
        if(p->data > pnew->data)
            break;
        pre = p;
        p = p->next;//找下一个
    }
    //中间插
    pnew->next = p;
    pnew->prev = pre;
    pre->next = pnew;
    p->prev = pnew;  
}
/*
	创建带头结点的双向链表
*/
dnode *create_circle_list()
{
    //创建一个头结点
    dnode *head = init_circle_list();
    
    delemtype d;
    while(1)
    {
        scanf("%d",&d);
        if(d == 0)
            break;
        
        //插入
        tail_insert_circle_list(head,d);
        //head_insert_circle_list(head,d);
        //sort_asc_insert_circle_list(head,d);
    }


    return head;
}

void print_circle_list(dnode *head)
{
    // dnode *p = head->next;
    // while(p!=head)
    // {
    //     printf("%d ",p->data);
    //     p = p->next;
    // }
    // printf("\n");

    // p = head->prev;
    // while(p!=head)
    // {
    //     printf("%d ",p->data);
    //     p = p->prev;
    // }
    // printf("\n"); 

    //参数有效性检查  
    if(head == NULL)
        return ;
    dnode *p = head->next;
    int i=0;
    printf("正序：");
    while(1)
    {
        if(p == head)
        {
            i++;
            if(i == 2)
                break;
            p = p->next;
        }
       
        printf("%d ",p->data);
        p = p->next;
    }
    printf("\n");
    printf("逆序：");
    p=head->prev;//保存了最后一个结点地址
    i=0;
    while(1)
    {
        if(p == head)
        {
            i++;
            if(i == 2)
                break;
            p = p->prev;
        }
       
        printf("%d ",p->data);
        p = p->prev;
    }
    printf("\n");
}


/*
	删除所有值为x的结点
*/
void delete_all_x_node_circle_list(dnode *head,delemtype x)
{
    if(head == NULL)
        return ;
    //1.找
    dnode *p = head->next;//p为查找的结点
    dnode *pre = head;

    while(1)
    {
        while(p!=head)
        {
            if(p->data == x)
                break;
            pre = p;
            p = p->next;//找下一个结点
        }

        //没有找到，已经删除完成
        if(p==head)
            break;

        //2.找到了删除
        if(p!=head)
        {

            pre->next = p->next;
            p->next->prev = pre;//如果要删除的是最后一个结点，这一行代码不能做的
            //孤立p结点
            p->next = p->prev = NULL;
            free(p);
            p = NULL;

        }

        p = pre->next;//p保存下一次要找的起始位置
    }

}

//请空链表中的数据，把所有的数据结点进行删除
void clean_circle_list(dnode *head)
{
    if(head == NULL)
        return ;
        
    dnode *p = head->next;//p保存要删除的节点

    while(p!=head)
    {
        head->next = p->next;
        p->next->prev = head;

        //孤立p结点
        p->next = p->prev = NULL;
        free(p);
        p = NULL;

        //p要继续保存要删除的节点
        p = head->next;
    }
}

/*
	销毁链表
*/
dnode* destroy_circle_list(dnode *head)
{
    if(head != NULL)
    {
        clean_circle_list(head);//清空了数据结点
        //删除头结点
        free(head);
        head = NULL;
    }
    
    return head;
}