#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
void *abc(void *arg)
{
    int num = 0;
    while(1)
    {
        num++;
        if(num == 10000)
        {
            //退出线程
            // int *a = malloc(sizeof(int));
            // *a = 10000;
            // pthread_exit((void*)a);
            pthread_exit((void*)-1);
        }
    }
}

int main()
{
    pthread_t id;//保存线程的id
    pthread_create(&id,NULL,abc,NULL);

    //等待线程退出
    int *exit_code;
    pthread_join(id,(void **)&exit_code);
    //printf("exit_code = %d\n",*exit_code);
    printf("exit_code = %d\n",exit_code);
}