#include "circle_list.h"
#include <stdio.h>

int main()
{
    //创建双向循环链表
    dnode *head = create_circle_list();
    print_circle_list(head);

    //删除值为x的所有结点
    delete_all_x_node_circle_list(head,5);
    print_circle_list(head);

    head = destroy_circle_list(head);
    print_circle_list(head);
}