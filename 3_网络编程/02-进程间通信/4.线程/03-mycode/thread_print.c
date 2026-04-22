#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

// 线程函数：打印hello
void *abc(void *arg)
{
    while (1)
    {
        printf("hello\n");
        sleep(1);
    }
}

int main()
{
    // 创建一个线程
    pthread_t thread;
    pthread_create(&thread, NULL, abc, NULL);

    // 主线程：打印world
    while (1)
    {
        printf("world\n");
        sleep(1);
    }

    return 0;
}