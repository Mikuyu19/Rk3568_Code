#include <stdio.h>
#include <stdlib.h>
#include "linklist_with_head.h"
//尾巴的性质：尾巴的下一个next
/*
	尾插法
	head:保存链表的头结点的地址
	d：需要插入的数据的值
	返回值：链表的头结点的地址
*/
node *tail_insert_with_head(node *head,elemtype d)
{

}
/*
    根据用户输入的顺序，创建带头结点的单链表
*/
node *create_list_with_head()
{
	//1.创建一个头结点
	node *head = malloc(sizeof(node));
	head->data = 0;//存储结点的数目
	head->next = NULL;
	
    elemtype d;
	while(1)
	{
		//输入数据
		scanf("%d",&d);
        if(d == 0)
            break;
		//创建结点
        node *pnew = (node*)malloc(sizeof(node));
        pnew->data = d;
        pnew->next = NULL;

		//找尾巴
        node *tail = head;//找尾巴的指针  尾巴的性质就是它的下一个为NULL

        while(tail -> next != NULL)
            tail = tail->next;

        //tail->next == NULL   尾巴找到了
        tail->next = pnew;

	}

    return head;
}
void print_list_with_head(node *head)
{
    if(head == NULL)
    {
        printf("链表不存在\n");
        return ;
    }
    node *p = head->next;

    while(p!=NULL)
    {
        printf("%d ",p->data);
        p = p->next;
    }
    printf("\n");

    //p为NULL  链表的数据已经打印完成了
}
/*
    删除所有值为x的结点
    head:保存链表的头结点的地址
    x：需要删除结点的值
*/
void delete_all_node_with_head(node*head,elemtype x)
{
    if(head == NULL)
        return ;

    //找
    node *p = head->next;//保存要找的起始位置
    node *pre = head;//保存p的前一个结点

    while(1)
    {
        while(p!=NULL)
        {
            if(p->data == x)
            {
                break;//找到了
            }
            else 
            {
                //没找到 访问下一个
                pre = p;
                p = p->next;
            }
        }

        if(p == NULL)//没找到
            break;

        if(p!= NULL)//找到了
        {
            //中间删
            pre->next = p->next;
            //孤立p结点
            p->next = NULL;
            //删除
            free(p);
            p = NULL;
        }

        p = pre->next;//保存下一次你要找的起始位置
    }

}

/*
    销毁链表，包括头结点
    head:保存链表的头结点的地址
*/
node *destroy_list_with_head(node *head)
{
    node *p = head;//p保存要删除的结点的地址

    while(p)
    {
        head = head->next;
        //孤立p
        p->next = NULL;
        free(p);
        p = NULL;

        p = head;//p保存下一次要删除的结点
    }

    return head;
}
/*
    创建带头结点的升序链表
*/
node *create_list_asc_with_head()
{
    //创建头结点
    node *head = (node*)malloc(sizeof(node));
    head->data = 0;
    head->next = NULL;

    while(1)
    {
        elemtype d;
        //输入数据
        scanf("%d",&d);
        if(d == 0)
            break;

        //创建结点
        node *pnew = (node*)malloc(sizeof(node));
        pnew->data = d;
        pnew->next = NULL;

        //找第一个比pnew大的结点
        node *p = head->next;//保存要查找的起始位置
        node *pre = head;//pre保存p的前一个的地址
        while(p!=NULL)
        {
            if(p->data > pnew->data)
            {
                break;
            }
            else
            {
                //没有找到,找下一个
                pre = p;
                p = p->next;
            }
        }

        //中间插
        pre->next = pnew;
        pnew->next = p;

    }
    return head;
}

void mins_ahead(node *head)
{
    if(head == NULL)
        return ;
    
    node *p = head->next;//指向负数的结点
    node *k = head->next;//指向待交换的结点

    while(p!=NULL)
    {
        if(p->data < 0)
        {
            //交换值
            elemtype temp = p->data;
            p->data = k->data;
            k->data = temp;

            //待交换的位置+1
            k = k->next;
        }
        p = p->next;
    }
}

void mins_ahead_v2(node *head)
{
    if(head == NULL)
        return ;
    
    node *p = head->next;//指向负数的结点
    node *pre = head;//保存p的前一个结点的地址

    while(1)
    {
        //找负数
        while(p!=NULL)
        {
            if(p->data < 0)
            {
                break;//找到了
            }
            else
            {
                //没有找到
                pre = p;
                p = p->next;
            }
        }

        //没有找到，找完了
        if(p==NULL)
            break;

        //找到了
        if(p!=NULL)
        {
            //摘
            pre->next = p->next;
            p->next = NULL;

            //插到head的后面
            p->next = head->next;
            head->next = p;
        }

        //p要继续找下一个负数
        p = pre->next;
    }
}

