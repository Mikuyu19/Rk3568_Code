#include <stdio.h>
#include "bst.h"

int main()
{
    //创建二叉排序树
    binode *r = create_bst();

    //打印
    print_order(r);
}