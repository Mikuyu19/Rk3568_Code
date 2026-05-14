#ifndef _CIRCLE_LIST_H_
#define _CIRCLE_LIST_H_

typedef int delemtype;
typedef struct dnode
{
	delemtype data;//数据域
	struct dnode *next;//指针域 保存下一个结点的地址
	struct dnode *prev;//指针域 保存上一个结点的地址
}dnode;

//创建一个新的结点
dnode *malloc_new_dnode(delemtype d);
/*
	尾插
*/
void tail_insert_circle_list(dnode *head,delemtype d);
/*
	头插
*/
void head_insert_circle_list(dnode *head,delemtype d);
/*
	有序插入
*/
void sort_asc_insert_circle_list(dnode *head,delemtype d);
/*
	创建带头结点的双向链表
*/
dnode *create_circle_list();

void print_circle_list(dnode *head);

/*
	删除所有值为x的结点
*/
void delete_all_x_node_circle_list(dnode *head,delemtype x);

/*
	销毁链表
*/
dnode* destroy_circle_list(dnode *head);
#endif