#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

// 互斥锁
pthread_mutex_t mutex;

void printer(char *str)
{
    // 加锁
    pthread_mutex_lock(&mutex);

    while (*str != '\0')
    {
        putchar(*str);
        fflush(stdout);
        sleep(1);
        str++;
    }
    putchar('\n');

    // 解锁
    pthread_mutex_unlock(&mutex);
}

void *thread1(void *arg)
{
    printer("hello");
    return NULL;
}

void *thread2(void *arg)
{
    printer("world");
    return NULL;
}

int main()
{
    pthread_t tid1, tid2;

    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&tid1, NULL, thread1, NULL);
    pthread_create(&tid2, NULL, thread2, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);

    return 0;
}
