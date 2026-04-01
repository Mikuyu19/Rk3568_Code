#ifndef _BST_H_
#define _BST_H_

//结点的类型定义
typedef int telemtype;
typedef struct binode
{
	telemtype data;//数据域
	struct binode *lchild;//左孩子
	struct binode *rchild;//右孩子
}binode;

//函数的声明
//插入结点
binode *insert_node(binode *r,telemtype d);
//创建一棵二叉排序树
binode *create_bst();
//打印
void print_order(binode *r);
#endif