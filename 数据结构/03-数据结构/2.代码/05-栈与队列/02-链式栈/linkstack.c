#include <stdio.h>
#include <stdlib.h>
#include "linkstack.h"

/*
    初始化操作，建立一个空栈s
    创建一个空栈，由s指向它
*/
linkstack *init_stack()
{
    //为s创建空间
    linkstack *s = malloc(sizeof(linkstack));
    //为你的s里面的东西进行赋值
    s->top = s->bottom = NULL;
    s->num = 0;
    //返回s
    return s;
}

/*
    若栈为空,返回true,否则返回false
*/
bool stack_is_empty(linkstack *s)
{
    //栈不存在 || 栈顶为NULL || 数据结点的个数为0
    if(s == NULL || s->top == NULL || s->num == 0)
        return true;
    
    return false;
}

/*
    将栈清空
    栈中没有数据就相当于清空
*/
void clean_stack(linkstack *s)
{
    //判断栈是否为空，如果为true ,直接返回
    if(stack_is_empty(s) == true)
        return ;

    //出栈(头删)
    while(s->top)
    {
        //1.为你要删除的结点起个名字
        node *p = s->top;
        //2.更新s->top
        s->top = p->next;
            //发现s->top == NULL,还需要s->bottom=NULL
        if(s->top == NULL)
            s->bottom = NULL;
        //3.孤立要删除的结点
        p->next = NULL;
        //4.删除
        free(p);
        p = NULL;
        //5.个数--
        s->num--;
    }
    
}
/*
    若栈存在，则销毁它
    释放栈s占用的存储空间
*/
linkstack *destroy_stack(linkstack *s)
{
    if(s!=NULL)
    {
        //清空栈
        clean_stack(s);
        free(s);
        s = NULL;
    }
    return s;
}

/* 
    入栈
    返回值
        true 可以入栈
        false 不可以入栈
*/
bool push(linkstack *s,selemtype e)
{
    //如果栈不存在 返回false
    if(s == NULL)
        return false;

    //栈存在  入栈(头插)

    //创建结点pnew
    node *pnew = malloc(sizeof(node));
    pnew->data = e;
    pnew->next = NULL;

    //头插
        //如果栈中没有数据结点，新的节点会成为你的top和bottom
    if(s->top == NULL)
    {
        s->top = s->bottom = pnew;
    }
        //如果有数据结点，做头插
    else
    {
        pnew->next = s->top;
        s->top = pnew;//更新top
    }
    
    //个数+1
    s->num++;
    return true;
}
/* 
    出栈
    返回值
        true 可以出栈
        false 不可以出栈

    调用
        selemtype e;
        pop(s,&e)
*/
bool pop(linkstack *s,selemtype *d)
{
    //判断栈是否为空，如果为true ,直接返回
    if(stack_is_empty(s) == true)
        return false;

    //出栈(头删)
    
    //获取数据
    *d = s->top->data;

    //1.为你要删除的结点起个名字
    node *p = s->top;
    //2.更新s->top
    s->top = s->top->next;
        //发现s->top == NULL,还需要s->bottom=NULL
    if(s->top == NULL)
        s->bottom = NULL;
    //3.孤立要删除的结点
    p->next = NULL;
    //4.删除
    free(p);
    p = NULL;
    //5.个数--
    s->num--;
    return true;
}

/*
    获取栈顶元素的值
    返回值
        true 可以获取栈顶
        false 不可以获取栈顶

    调用
        selemtype e;
        get_stack_top(s,&e)
*/
bool get_stack_top(linkstack *s,selemtype *d)
{
    //判断栈为空
    if(stack_is_empty(s))
        return false;
    //能获取
    *d = s->top->data;
    return true;

}

/*
    获取栈中元素的个数
*/
int get_stack_length(linkstack *s)
{
    if(s==NULL)
        return 0;
    return s->num;
}