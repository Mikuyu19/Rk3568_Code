//linkst.c
#include "linklist.h"
#include <stdio.h>
#include <stdlib.h>

//尾巴的性质：尾巴的下一个next
/*
	尾插法
	head:保存链表的第一个节点的地址
	d：需要插入的数据的值
	返回值：返回链表的第一个节点的地址
*/
node *tail_insert(node *head,elemtype d)
{
	//1.为d创建一个新的结点
    node *pnew = (node*)malloc(sizeof(node));
    pnew->data = d;
    pnew->next = NULL;    
	
	//2.插入
    if(head == NULL)//没有数据结点
    {
    	head=pnew;//新的结点会成为头
    }
    else//有数据结点
    {
    	//2.1 找尾巴tail
        node *tail = head;
        while(tail->next != NULL)
            tail = tail->next;
    	
        //跳出来tail一定是最后一个结点
    	//2.2 把pnew插入到最后
        tail->next = pnew;//最后一个结点的下一个为pnew
    }	
    
    //返回第一个结点的地址
    return head;
}

/*
	头插法
	head:保存链表的第一个节点的地址
	d：需要插入的数据的值
	返回值：返回链表的第一个节点的地址
*/
node *head_insert(node *head,elemtype d)
{
	//1.为d创建一个新的结点
    node *pnew = (node*)malloc(sizeof(node));
    pnew->data = d;
    pnew->next = NULL;    
		
	//2.插入(头插法)
	if(head == NULL)
	{
		head=pnew;//新的结点会成为头
	}
	else
	{
		pnew->next = head;//新结点的下一个是head
		head = pnew;//新结点成为第一个结点
	}
}

node *add_a_node(node *head,elemtype x,elemtype a)
{
	//1.为a创建一个新的结点
    node *pnew = (node*)malloc(sizeof(node));
    pnew->data = a;
    pnew->next = NULL;   

	//2.找值为x的结点(遍历)
	node *p = head;//p找值为x的结点
	node *pre = NULL;//保存p的前一个结点
	while(p!=NULL)
	{
        if(p->data == x)//找到了
			break;
        //没找到
        pre = p;//p会成为前一个结点
        p=p->next;//找下一个结点
	}
	
	//3.找到了if(p!=NULL)
    if(p!=NULL)
    {
		//3.1 p==head  头插
        if(p == head)
        {
            pnew->next = head;
            head = pnew;
        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
        else //3.2 p!=head  中间插   
        {
            pnew->next = p;
            pre->next = pnew;
        }
    }
	//4.没找到  
    else
    {
        if(head == NULL)//链表中没有结点
        {
            head = pnew;
        }
        else//尾插
        {
            //pre是尾巴
            pre->next = pnew;
        }
    }
	
	return head;
}
//创建链表
node *create_list()
{
	node *head = NULL;//头指针
	node *tail = NULL;//尾指针
	
	while(1)
	{
        elemtype d;
        //1.输入数据	(约定输入0结束循环)
        scanf("%d",&d);
        if(d == 0)
            break;
        // //2.为你的数据创建一个节点
        // node *pnew = (node*)malloc(sizeof(node));
        // pnew->data = d;
        // pnew->next = NULL;
        // //3.把新的结点插入到链表中(尾插法)
        // if(head == NULL)//没有数据结点
        // {
        //     head=tail=pnew;//新的结点会成为头和尾
        // }
        // else//有数据结点
        // {
        //     tail->next = pnew;//tail的下一个为pnew
        //     tail = pnew;//pnew成为尾巴
        // }

        head = tail_insert(head,d);
        //head = head_insert(head,d);
	}
	
	return head;
}

//打印链表
void print_list(node *head)
{
    node *temp = head;//移动指针

    while(temp!=NULL)
    {
        printf("%d ",temp->data);//打印当前这个元素的数据
        temp = temp->next;//访问temp的下一个元素	
    }
    printf("\n");
}


node *delete_node(node*head,elemtype x)
{
	//1.找值为x的结点
	node *p = head;//用来找值为x的结点
    node *pre = NULL;//保存x结点的前一个
    while(p!=NULL)
    {
        if(p->data != x)
        {
            //找下一个
            pre = p;
            p= p->next;
        }
        else 
        {
            break;//找到了
        }
    }
	//2.删除值为x的结点(分情况讨论 头删 中间删和尾删统一为中间删的代码)
	//找到了 if(p!=NULL)
    if(p!=NULL)
    {
        //情况1：找到的节点p就是head  头删
        if(p == head)
        {
            head = head->next;
        }
        //情况2：中间删
        else 
        {
            pre->next = p->next;             
        }
        //孤立p
        p->next = NULL;
        //删除
        free(p);
        p = NULL;
    }

    return head;
}

//循环头删 把每一个结点都要释放麻醉后打印链表的结果为NULL
node* destroy_list(node *head)
{
    node *p = head;//p保存要删除的结点的地址

    while(p)
    {
        head = head->next;
        //孤立p
        p->next = NULL;
        free(p);
        p = NULL;

        p = head;//p保存下一次要删除的结点
    }

    return head;
}

//循环头删 把每一个结点都要释放，最后打印链表的结果为NULL
void destroy_list_v2(node **head)
{
    node *p = *head;//p保存要删除的结点的地址

    while(p)
    {
        *head = (*head)->next;
        //孤立p
        p->next = NULL;
        free(p);
        p = NULL;

        p = *head;//p保存下一次要删除的结点
    }
}

//递归版本
node* destroy_list_v3(node *head)
{
    if(head == NULL)//没有结点
        return NULL;
    else
    {
        node *p = head;
        head = head->next;
        //删p
        p->next = NULL;
        free(p);
        p = NULL;
        return destroy_list_v3(head);
    }
}


node *create_list_asc()
{
	node *head = NULL;
	
	while(1)
	{
		//1.输入数据(约定输入0结束循环)
        elemtype d;
        scanf("%d",&d);
        if(d == 0)
            break;
		
		//2.创建结点pnew
        node *pnew = (node*)malloc(sizeof(node));
        pnew->data = d;
        pnew->next = NULL;
		
		//3.把新的结点插入到到链表中
		//3.1 找位置 找第一个比pnew大的结点p
        node *p = head;
        node *pre = NULL;//p的前一个结点的地址
        while(p!=NULL)
        {
            if(p->data > pnew->data)
                break;
            else //没找到
            {
                pre = p;
                p = p->next;//找下一个
            }
        }

		//3.2 插入
		//如果找到了 头插(p==head)  中间插
        if(p!=NULL)
        {
            if(p == head)//头插
            {
                pnew->next = p;//新的结点的下一个结点是head
                head = pnew;//pnew会成为第一个结点
            }
            else
            {
                pnew->next = p;
                pre->next = pnew;
            }
        }
        else//如果没找到  
        {
            if(head == NULL)//链表中没有节点
            {
                head = pnew;
            }
            else//尾插
            {
                //pre是最后一个结点
                pre->next = pnew;
            }
        }
		
	}
	
	return head;
}


int get_node_count(node *head)
{
    node *p = head;//p是遍历指针
    int count = 0;
    while(p !=NULL)
    {
        count++;//个数+1
        p=p->next;//找下一个结点
    }
    return count;
}

node *reverse_list(node *head)
{
	node *h = NULL;//指向新的链表
	node *p = head;//p保存要摘下来的那个节点
	
	while(head != NULL)
	{
        //1.摘下来
        p=head;
        head = head->next;
        p->next = NULL;
        //2.把节点p使用头插 放到h链表中	
        if(h == NULL)
        	h = p;
        else
        {
            p->next = h;
            h = p;
        }		
	}
	
	return h;
}

elemtype get_k_value(node*h)
{
    int k;
    printf("请输入k:");
    scanf("%d",&k);

    node *fast = h;//快
    node *slow = h;//慢

    //先让fast走k步
    int i = 0;
    for(i=0;i<k && fast;i++)
    {
        fast = fast->next;
    }

    if(fast == NULL)
    {
        printf("k is error\n");
        return -1;
    }

    while(fast)
    {
        fast = fast->next;
        slow = slow->next;
    }

    //跳出的时候slow就是倒数第k个结点
    return slow->data;
}

elemtype get_middle_value(node*h)
{
    node *fast = h;
    node *slow = h;
    //fast每次走两步 slow每次走一步
    while(fast)
    {
        fast = fast->next;
        if(fast == NULL)
            break;

        fast = fast->next;
        if(fast == NULL)
            break;

        slow = slow->next;
    }

    return slow->data;
}

/*
    1 有环
    0 没有环
*/
int is_ring(node*h)
{
    node *fast = h;
    node *slow = h;

    while(fast)
    {
        fast = fast->next;
        if(fast == NULL)
            break;

        fast = fast->next;
        if(fast == NULL)
            break;

        slow = slow->next;

        if(fast == slow)//相遇了 有环
            return 1;
    }  
    
    //无环
    return 0;
}

//求交集
node*common_of_list(node *a,node*b)
{

    //创建一个新的链表c(交集)
    node *c = NULL;//一开始新的链表没有数据结点

    //遍历A和B
    int temp = -1;//保存上一次相同结点的值
    while(a && b)
    {
        //找相同的节点
        if(a->data == b->data)
        {
            //相同且不重复
            if(a->data!=temp)
            {
                c = tail_insert(c,a->data);

                //记录插入到c链表的值
                temp = a->data;
            }
            a = a->next;
            b = b->next;
        }
        else if(a->data > b->data)//b小b往后走
        {
            b = b->next;
        }
        else//a小a往后走
        {
            a = a->next;
        }
    }

    return c;
}


node *bingji(node *a,node *b)
{
    //创建新链表C
    node *c = NULL;//一开始新的链表没有数据结点

    //遍历A和B
    while(a && b)
    {
        //找相同你的节点
        if(a->data == b->data)
        {
            //创建两个相同的结点
            c = tail_insert(c,a->data);
            c = tail_insert(c,a->data);
            a = a->next;
            b = b->next;
        }
        else if (a->data < b->data)
        {
            c = tail_insert(c,a->data);
            a = a->next;
        }
        else
        {
            c = tail_insert(c,b->data);
            b = b->next;
        }
    }

    while(a)
    {
        //尾插
        c= tail_insert(c,a->data);
        a = a->next;
    }

    while(b)
    {
        //尾插
        c= tail_insert(c,b->data);
        b = b->next;
    }

    return c;
}




