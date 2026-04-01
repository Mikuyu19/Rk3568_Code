#include <stdio.h>
#include <stdlib.h>
#include "bst.h"
#include "linkqueue.h"
//插入结点
binode *insert_node(binode *r,telemtype d)
{
	if(r == NULL)//空树
	{
		//新的结点成为你的根结点
        binode *pnew = malloc(sizeof(binode));
        pnew->data = d;
        pnew->lchild = pnew->rchild = NULL;
        
        r = pnew;
	}
    else
    {
        binode *p = r;//p找
        while(1)
        {
            if(p->data > d)//往左找
            {
                //如果p的左子为空， 新的结点就会成为p的左子  直接返回
                if(p->lchild == NULL)
                {
                    //创建新结点
                    binode *pnew = malloc(sizeof(binode));
                    pnew->data = d;
                    pnew->lchild = pnew->rchild = NULL;
                    //插入
                    p->lchild = pnew;//已经插入了，循环结束
                    break;
                }
                //否则p要往左走
                p = p->lchild;
            }
            else if(p->data < d)//往右找
            {
                //如果p的右子为空， 新的结点就会成为p的右子  直接返回
                if(p->rchild == NULL)
                {
                    //创建新结点
                    binode *pnew = malloc(sizeof(binode));
                    pnew->data = d;
                    pnew->lchild = pnew->rchild = NULL;
                    //插入
                    p->rchild = pnew;//已经插入了，循环结束
                    break;
                }
                //否则p要往右走
                p = p->rchild;
            }
            else	//相等
            {
                break;
            }
        }
    }

    return r;

}

//插入结点  递归
binode *insert_node_v2(binode *r,telemtype d)
{
	if(r == NULL)//空树
	{
		//新的结点成为你的根结点
        binode *pnew = malloc(sizeof(binode));
        pnew->data = d;
        pnew->lchild = pnew->rchild = NULL;
        
        r = pnew;
        return r;
	}
	
    if(r->data > d)//往左找
    {
        r->lchild = insert_node_v2(r->lchild,d);
    }
    else if(r->data < d)//往右找
    {
        r->rchild = insert_node_v2(r->rchild,d);
    }

    return r;

}

//创建一棵二叉排序树
binode *create_bst()
{
	binode *r = NULL;//保存根结点
	
    telemtype d;
	while(1)
	{
		//输入数据
        scanf("%d",&d);
        if(d == 0)
            break;
		
		//插入
        r = insert_node(r,d);
	}
	
	//返回根结点
    return r;
}

//先序
void pre_order(binode *r)
{
    if(r!=NULL)
    {
        printf("%d ",r->data);//访问根结点
        pre_order(r->lchild);//按照先序遍历的方式去访问根的左子树
        pre_order(r->rchild);//按照先序遍历的方式去访问根的右子树
    }
}

//中序
void mid_order(binode *r)
{
    if(r!=NULL)
    {
        mid_order(r->lchild);//按照中序遍历的方式去访问根的左子树
        printf("%d ",r->data);//访问根结点
        mid_order(r->rchild);//按照中序遍历的方式去访问根的右子树
    }
}

//后序
void post_order(binode *r)
{
    if(r!=NULL)
    {
        post_order(r->lchild);//按照后序遍历的方式去访问根的左子树
        post_order(r->rchild);//按照后序遍历的方式去访问根的右子树
        printf("%d ",r->data);//访问根结点
    }
}

void level_order(binode*r)
{
	//1.初始化一个队列
    linkqueue *q = init_queue();
	
	//2.根节点入队
    enqueue(q,r);
	
    qelemtype e;
	while(!queue_is_empty(q))//队列不为空
	{
		//3.出队 出队元素进行访问
        dequeue(q,&e);
        printf("%d ",e->data);
		
		//4.把这个出队结点的左子和右子全部入队
        if(e->lchild)
            enqueue(q,e->lchild);
        if(e->rchild)
            enqueue(q,e->rchild);
	}
	
	//5.销毁队列
    q = destroy_queue(q);
}

//打印
void print_order(binode *r)
{
    //先序
    pre_order(r);
    printf("\n");
    //中序
    mid_order(r);
    printf("\n");
    //后序
    post_order(r);
    printf("\n");
    //层次
    level_order(r);
    printf("\n");

}