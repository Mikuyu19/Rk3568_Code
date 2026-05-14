#include <stdio.h>
#include "double_list.h"
#include <stdlib.h>
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
void tail_insert_double_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //找尾巴
    dnode *tail = head;
    while(tail->next != NULL)
    {
        tail = tail->next;
    }
    //tail就是尾巴

    //尾插
    tail->next = pnew;
    pnew->prev = tail;
}
/*
	头插
*/
void head_insert_double_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //头插
    pnew->next = head->next;
    pnew->prev = head;
    head->next = pnew;
    if(pnew->next!=NULL)
        pnew->next->prev = pnew;//链表中没有结点，新添加一个结点，不能做这行代码

}
/*
	有序插入
*/
void sort_asc_insert_double_list(dnode *head,delemtype d)
{
    //参数有效性检查
    if(head == NULL)
        return ;

    //创建一个新的结点
    dnode *pnew = malloc_new_dnode(d);

    //找第一个比pnew大的结点
    dnode *p = head->next;
    dnode *pre = head;
    while(p)
    {
        if(p->data > pnew->data)
            break;
        pre = p;
        p = p->next;//找下一个
    }

    //插入
    if(p!=NULL)//找到了
    {
        //中间插
        pnew->next = p;
        pnew->prev = pre;
        pre->next = pnew;
        p->prev = pnew;
    }
    else//没有找到
    {
        //尾插
        pre->next=pnew;
        pnew->prev = pre;
    }
    /*
        pnew->next = p;
        pnew->prev = pre;
        pre->next = pnew;
        if(p!=NULL)
            p->prev = pnew;        
    */
}
/*
	创建带头结点的双向链表
*/
dnode *create_double_list()
{
    //创建一个新的结点
    dnode *head = malloc_new_dnode(0);
    
    delemtype d;
    while(1)
    {
        scanf("%d",&d);
        if(d == 0)
            break;
        
        //插入
        tail_insert_double_list(head,d);
        //head_insert_double_list(head,d);
        //sort_asc_insert_double_list(head,d);
    }

    //首尾相接

    return head;
}

void print_double_list(dnode *head)
{
    dnode *p = head->next;
    dnode *pre = head;
    while(p)
    {
        printf("%d ",p->data);
        pre = p;
        p = p->next;
    }
    printf("\n");
    //pre就是最后一个结点
    while(head!=pre)
    {
        printf("%d ",pre->data);
        pre = pre->prev;
    }
    printf("\n");
}


/*
	删除所有值为x的结点
*/
void delete_all_x_node_double_list(dnode *head,delemtype x)
{
    if(head == NULL)
        return ;
    //1.找
    dnode *p = head->next;//p为查找的结点
    dnode *pre = head;

    while(1)
    {
        while(p!=NULL)
        {
            if(p->data == x)
                break;
            pre = p;
            p = p->next;//找下一个结点
        }

        //没有找到，已经删除完成
        if(p==NULL)
            break;

        //2.找到了删除
        if(p!=NULL)
        {
            /*
            //中间删除
            pre->next = p->next;
            p->next->prev = pre;
            //孤立p结点
            p->next = p->prev = NULL;
            free(p);
            p = NULL;

            //尾删
            pre->next = NULL;

            p->prev = NULL;
            free(p);
            p = NULL;
            */
            pre->next = p->next;
            if(p->next!=NULL)
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
void clean_double_list(dnode *head)
{
    if(head == NULL)
        return ;
        
    dnode *p = head->next;//p保存要删除的节点

    while(p!=NULL)
    {
        head->next = p->next;
        if(p->next!=NULL)
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
dnode* destroy_double_list(dnode *head)
{
    if(head != NULL)
    {
        clean_double_list(head);//清空了数据结点
        //删除头结点
        free(head);
        head = NULL;
    }
    
    return head;
}