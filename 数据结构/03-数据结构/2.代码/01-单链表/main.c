//main.c
#include <stdio.h>
#include "linklist.h"
int main()
{
	// //创建链表
	// node *head = create_list();
	// //打印链表
	// print_list(head);	
    // //在值为x的结点前面插入一个a
    // elemtype x,a;
    // printf("请输入x和a的值:");
    // scanf("%d%d",&x,&a);
    // head = add_a_node(head,x,a);
	// //打印链表
	// print_list(head);

    // //删除一个结点
    // elemtype x;
    // printf("请输入x的值:");
    // scanf("%d",&x);
    // head = delete_node(head,x);
	// //打印链表
	// print_list(head);  
    
    // //销毁一个链表
    // destroy_list_v2(&head);
    // print_list(head);

 	// //创建链表
	// node *head = create_list_asc();
	// //打印链表
	// print_list(head);   

    // printf("个数为%d\n",get_node_count(head));

    // //逆置链表
    // head = reverse_list(head);
    // //打印链表
	// print_list(head);  
    
    //创建两条链表
    node *a = create_list_asc();
    node *b = create_list_asc();
    print_list(a);
    print_list(b);

    //求交集
    node *c = common_of_list(a,b);
    print_list(c);

    node *d = bingji(a,b);
    print_list(d);
}