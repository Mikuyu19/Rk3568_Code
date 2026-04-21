#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// 线程函数1：无参数
void *thread_func1(void *arg)
{
    printf("线程1开始执行，线程ID: %lu\n", pthread_self());

    for (int i = 0; i < 5; i++)
    {
        printf("线程1: 计数 %d\n", i);
        sleep(1);
    }

    printf("线程1结束\n");
    return NULL;
}

// 线程函数2：带参数
void *thread_func2(void *arg)
{
    int num = *(int *)arg;  // 获取传入的参数
    printf("线程2开始执行，线程ID: %lu，接收参数: %d\n", pthread_self(), num);

    for (int i = 0; i < 5; i++)
    {
        printf("线程2: 计数 %d\n", i);
        sleep(1);
    }

    printf("线程2结束\n");
    return NULL;
}

// 线程函数3：带返回值
void *thread_func3(void *arg)
{
    printf("线程3开始执行，线程ID: %lu\n", pthread_self());

    // 动态分配返回值（必须用堆内存，栈内存会被释放）
    int *result = (int *)malloc(sizeof(int));
    *result = 100;

    for (int i = 0; i < 3; i++)
    {
        printf("线程3: 计数 %d\n", i);
        (*result) += i;
        sleep(1);
    }

    printf("线程3结束，返回值: %d\n", *result);
    pthread_exit(result);  // 返回结果
}

int main()
{
    pthread_t tid1, tid2, tid3;
    int param = 888;

    printf("主线程开始，线程ID: %lu\n", pthread_self());

    // 1. 创建线程1（无参数）
    if (pthread_create(&tid1, NULL, thread_func1, NULL) != 0)
    {
        perror("创建线程1失败");
        return -1;
    }
    printf("成功创建线程1，ID: %lu\n", tid1);

    // 2. 创建线程2（带参数）
    if (pthread_create(&tid2, NULL, thread_func2, &param) != 0)
    {
        perror("创建线程2失败");
        return -1;
    }
    printf("成功创建线程2，ID: %lu\n", tid2);

    // 3. 创建线程3（带返回值）
    if (pthread_create(&tid3, NULL, thread_func3, NULL) != 0)
    {
        perror("创建线程3失败");
        return -1;
    }
    printf("成功创建线程3，ID: %lu\n", tid3);

    printf("\n主线程等待子线程结束...\n\n");

    // 4. 等待线程1结束
    pthread_join(tid1, NULL);
    printf("线程1已回收\n");

    // 5. 等待线程2结束
    pthread_join(tid2, NULL);
    printf("线程2已回收\n");

    // 6. 等待线程3结束并获取返回值
    int *ret_val;
    pthread_join(tid3, (void **)&ret_val);
    printf("线程3已回收，返回值: %d\n", *ret_val);
    free(ret_val);  // 释放线程返回的堆内存

    printf("\n所有线程已结束，主线程退出\n");

    return 0;
}
