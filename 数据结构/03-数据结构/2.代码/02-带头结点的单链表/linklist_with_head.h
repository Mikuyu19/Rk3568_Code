//linklist.h
#ifndef _LINKLIST_WITH_HEAD_H_
#define _LINKLIST_WITH_HEAD_H_

typedef int elemtype;//给int创建一个别名叫做elemtype
typedef struct node
{
	elemtype data;
	struct node*next;
}node;

/*
    根据用户输入的顺序，创建带头结点的单链表
*/
node *create_list_with_head();
/*
    打印带头结点的单链表
*/
void print_list_with_head(node *head);
//尾巴的性质：尾巴的下一个next
/*
	尾插法
	head:保存链表的头结点的地址
	d：需要插入的数据的值
	返回值：链表的头结点的地址
*/
node *tail_insert_with_head(node *head,elemtype d);
/*
    删除所有值为x的结点
    head:保存链表的头结点的地址
    x：需要删除结点的值
*/
void delete_all_node_with_head(node*head,elemtype x);

/*
    销毁链表，包括头结点
    head:保存链表的头结点的地址
*/
node *destroy_list_with_head(node *head);
/*
    创建带头结点的升序链表
*/
node *create_list_asc_with_head();

void mins_ahead(node *head);

#endif