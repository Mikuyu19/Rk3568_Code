#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

// 线程函数：设置为不能被取消
void *abc(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    printf("线程设置为不能被取消\n");
    int i = 0;
    while (1)
    {
        printf("hello %d\n", i++);
        sleep(1);
        if (i >= 10)
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

    for (int i = 0; i < 5; i++)
    {
        printf("world %d\n", i);
        sleep(1);
    }
    pthread_cancel(thread);

    pthread_join(thread, NULL);

    return 0;
}
