#include <stdio.h>

typedef struct node //数据结点
{
    int data;//数据域
    struct node *next;//指针域
}node;

int main()
{
    //创建4个结点
    node a,b,c,d;
    //为每个结点赋值以及关系的建立
    (&a)->data = 1;
    (&a)->next = &b;
    (&b)->data = 3;
    (&b)->next = &c;
    (&c)->data = 5;
    (&c)->next = &d;
    (&d)->data = 7;
    (&d)->next = NULL;    
    //定义一个head指针保存第一个元素地址
    node *head = &a;   //head = &a

    /*
        //打印
        printf("%d\n",(&a)->data);//关系：head = &a   得到head->data
        printf("%d\n",(&b)->data);//关系：(&a)->next = &b; ====> (&a)->next->data ==> head->next->data
        printf("%d\n",(&c)->data);//关系：(&b)->next = &c; ====> (&b)->next->data ==> (&a)->next->next->data ==> head->next->next
        printf("%d\n",(&d)->data);

        node *temp = head;//移动指针
        printf("%d ",temp->data);//1
        temp = temp->next;//temp=head->next ===> temp = &b
        printf("%d ",temp->data);//3
        temp = temp->next;//temp=(&b)->next ===> temp = &c
        printf("%d ",temp->data);//5
        temp = temp->next;//temp=(&c)->next ===> temp = &d
        printf("%d ",temp->data);//7
    */
    node*temp = head;
    while(temp!=NULL)
    {
        //重复的代码
        printf("%d ",temp->data);
        temp = temp->next;//访问下一个元素
    }


}