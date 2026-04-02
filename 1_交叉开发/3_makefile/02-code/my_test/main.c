#include <stdio.h>
#include <stdlib.h>
#include "bst.h" // 必须包含你的库头文件

int main()
{

    // 1. 调用库文件中的函数创建二叉树
    binode *root = create_bst();

    // 2. 调用库文件中的打印函数
    // 你的 print_order() 会自动依次调用先序、中序、后序、层次遍历的打印
    print_order(root);


    return 0;
}