#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void printer(char *str)
{
    while (*str != '\0')
    {
        putchar(*str);
        fflush(stdout);
        sleep(1);
        str++;
    }
    putchar('\n');
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

    pthread_create(&tid1, NULL, thread1, NULL);
    pthread_create(&tid2, NULL, thread2, NULL);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    return 0;
}
