//linklist.h
#ifndef _LINKLIST_H_
#define _LINKLIST_H_

typedef int elemtype;//给int创建一个别名叫做elemtype
typedef struct node
{
	elemtype data;
	struct node*next;
}node;

node *create_list();
void print_list(node *head);
//尾巴的性质：尾巴的下一个next
/*
	尾插法
	head:保存链表的第一个节点的地址
	d：需要插入的数据的值
	返回值：返回链表的第一个节点的地址
*/
node *tail_insert(node *head,elemtype d);

/*
	头插法
	head:保存链表的第一个节点的地址
	d：需要插入的数据的值
	返回值：返回链表的第一个节点的地址
*/
node *head_insert(node *head,elemtype d);

node *add_a_node(node *head,elemtype x,elemtype a);

node *delete_node(node*head,elemtype x);

node *destroy_list(node *head);


//循环头删 把每一个结点都要释放麻醉后打印链表的结果为NULL
void destroy_list_v2(node **head);

node *create_list_asc();


int get_node_count(node *head);

node *reverse_list(node *head);

//求交集
node*common_of_list(node *a,node*b);

node *bingji(node *a,node *b);

#endif