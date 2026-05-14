#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void *thread_func(void *arg)
{
    int *num = (int *)malloc(sizeof(int));
    *num = 0;
    while (*num < 10000)
    {
        (*num)++;
    }

    pthread_exit(num);
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, thread_func, NULL);

    // 等待线程结束并获取返回值
    int *ret;
    pthread_join(tid, (void **)&ret);
    printf("线程返回值: %d\n", *ret);
    free(ret); // 释放线程分配的内存
}