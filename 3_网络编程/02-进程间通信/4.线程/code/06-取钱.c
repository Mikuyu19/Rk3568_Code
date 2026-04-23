#include <pthread.h>
#include <stdio.h>
#include <signal.h>

int money = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

//孩子
void *child(void *arg)//void *arg = &i  => arg = &i
{
    pthread_mutex_lock(&mutex2);
    int i = *(int*)arg;
    //发送唤醒信号
    pthread_cond_signal(&cond2);
    pthread_mutex_unlock(&mutex2);
    int qu_money = 1000;

    //p
    pthread_mutex_lock(&mutex);
    while(money<1000)//若余额不足，则进入等待睡眠，顺序解锁
    {
        pthread_cond_wait(&cond,&mutex);
    }
    //取钱
    money-=qu_money;
    printf("t%d已经取走%d元,余额为%d元\n",i,qu_money,money);

    //v
    pthread_mutex_unlock(&mutex);
}

int main()
{

    //初始化线程互斥锁
    pthread_mutex_init(&mutex,NULL);
    //初始化条件变量
    pthread_cond_init(&cond,NULL);

    //父母 循环创建若干线程
    int thread_count = 3;//3个孩子
    pthread_t id[thread_count];
    for(int i = 0;i<thread_count;i++)
    {
        //p
        pthread_mutex_lock(&mutex2);
        pthread_create(&id[i],NULL,child,(void*)&i);
        //等待
        pthread_cond_wait(&cond2,&mutex2);//等待
        //v
        pthread_mutex_unlock(&mutex2);
    }

    //p
    pthread_mutex_lock(&mutex);

    //汇钱
    money=money+1000*thread_count;

    //通知所有正在等待的线程
    pthread_cond_broadcast(&cond);

    //v
    pthread_mutex_unlock(&mutex);


    for(int i = 0;i<thread_count;i++)
    {
        pthread_join(id[i],NULL);
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("任务已完成\n");
}