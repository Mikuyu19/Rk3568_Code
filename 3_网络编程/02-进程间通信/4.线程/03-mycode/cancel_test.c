#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

// 线程函数：打印hello并可以被取消
void *abc(void *arg)
{
    int count = 0;
    while (1)
    {
        printf("hello %d\n", count++);
        sleep(1);
        if (count >= 10)
        {
            printf("线程主动退出\n");
            pthread_exit(NULL);
        }
    }
}

int main()
{
    // 创建一个线程
    pthread_t thread;
    pthread_create(&thread, NULL, abc, NULL);

    // 主线程：打印5次world后取消线程
    for (int i = 0; i < 5; i++)
    {
        printf("world %d\n", i);
        sleep(1);
    }
    pthread_cancel(thread);
    pthread_join(thread, NULL);

    return 0;
}
