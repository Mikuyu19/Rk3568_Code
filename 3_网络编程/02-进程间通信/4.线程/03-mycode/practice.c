#include <stdio.h>
#include <pthread.h>

int data = 0;
// 互斥锁和条件变量
pthread_mutex_t mutex;
pthread_cond_t cond;

// 子线程函数
void *thread_func(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);

        // 等待条件满足：data >= 100000000
        while (data < 100000000)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        printf("完成小目标\n");
        data = 0;

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    pthread_t tid;

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 创建子线程
    pthread_create(&tid, NULL, thread_func, NULL);

    // 主线程循环累加data
    while (1)
    {
        pthread_mutex_lock(&mutex);
        data++;

        // 当达到目标值时，通知等待的线程
        if (data >= 100000000)
        {
            pthread_cond_signal(&cond);
        }

        pthread_mutex_unlock(&mutex);
    }

    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
