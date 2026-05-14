#include <stdio.h>
#include "linklist_with_head.h"
int main()
{
    node *head = create_list_with_head();
    print_list_with_head(head);

    mins_ahead(head);
    print_list_with_head(head);

    // delete_all_node_with_head(head,3);
    // print_list_with_head(head);

    // head = destroy_list_with_head(head);
    // print_list_with_head(head);
    // node *head2 = create_list_asc_with_head();
    // print_list_with_head(head2);    
}