#include <stdio.h>
#include <stdlib.h>
#include "bst.h" // 必须包含你的库头文件

int main()
{
    printf("========== 二叉排序树动态库测试 ==========\n");
    printf("请依次输入整数构建二叉排序树 (输入 0 结束):\n");

    // 1. 调用库文件中的函数创建二叉树
    // 因为你的 create_bst() 内部已经有 scanf 循环，程序运行到这里会停下来等你输入数据
    binode *root = create_bst();

    // 防止什么都没输入直接输入0的情况
    if (root == NULL)
    {
        printf("未输入有效节点，树为空。\n");
        return 0;
    }

    printf("\n========= 打印遍历结果 =========\n");
    // 2. 调用库文件中的打印函数
    // 你的 print_order() 会自动依次调用先序、中序、后序、层次遍历的打印
    print_order(root);

    printf("========== 测试圆满结束 ==========\n");
    return 0;
}