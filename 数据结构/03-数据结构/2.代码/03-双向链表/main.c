#include "double_list.h"
#include <stdio.h>

int main()
{
    //创建双向链表
    dnode *head = create_double_list();
    print_double_list(head);

    //删除值为x的所有结点
    delete_all_x_node_double_list(head,5);
    print_double_list(head);

    head = destroy_double_list(head);
    print_double_list(head);
}