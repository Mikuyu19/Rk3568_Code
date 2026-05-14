#include <pthread.h>
#include <stdio.h>
#include <signal.h>

int data = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;
int flag  = 1;
//消费者
void *thread(void *arg)
{
    while(flag)
    {
        //p
        pthread_mutex_lock(&mutex);

        while(data < 100000000 && flag)
        {
            //等待一个条件变量
            printf("消费者正在等待条件变量\n");
            pthread_cond_wait(&cond,&mutex);
            printf("消费者已经被唤醒\n");
            
        }
        printf("完成小目标\n");
        data = 0;
        printf("消费者唤醒一个生产者线程\n");
        pthread_cond_signal(&cond);        
        //v
        pthread_mutex_unlock(&mutex);
    }
    printf("消费者线程退出\n");
    pthread_cond_broadcast(&cond);
}

//生产者
void *thread2(void *arg)
{
    while(flag)
    {
        //p
        pthread_mutex_lock(&mutex);
        while(data >= 100000000 && flag)
        {
            //生产者等待一个条件变量
            printf("生产者正在等待条件变量\n");
            pthread_cond_wait(&cond,&mutex);
            printf("生产已经被唤醒\n");            
        }
        data++;
        if(data >= 100000000)
        {
            printf("生产者唤醒一个消费者线程\n");
            pthread_cond_signal(&cond);
        }
        //p
        pthread_mutex_unlock(&mutex);
    }
    printf("生产者线程退出\n");
    pthread_cond_broadcast(&cond);
}

void handler(int signum)
{
    flag = 0;
}
int main()
{
    signal(SIGINT,handler);
    //初始化线程互斥锁
    pthread_mutex_init(&mutex,NULL);
    //初始化条件变量
    pthread_cond_init(&cond,NULL);

    //开辟线程 消费者
    pthread_t id;
    pthread_create(&id,NULL,thread,NULL);

    //生产者
    pthread_t id2;
    pthread_create(&id2,NULL,thread2,NULL);


    pthread_join(id,NULL);
    pthread_join(id2,NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("任务已完成\n");
}