#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//打印机资源
void printer(char *str)
{
	//打印str中的每一个字符
	//想要一个字符一个字符显示的效果，可以一秒钟打印一个字符
    int i = 0;
    while(str[i])
    {
        putchar(str[i]);
        fflush(stdout);//冲刷流
        sleep(1);   
        i++;     
    }
    printf("\n");
	/*
		eg：
			putchar('a');
			fflush(stdout);//冲刷流
			sleep(1);
	*/
}

#if 1
//自己写两个线程hanshu 
void *thread1(void *arg)//void *arg=&mutex2  => arg=&mutex2
{
    while(1)
    {
        pthread_mutex_lock((pthread_mutex_t *)arg);
        printer("hello");
        pthread_mutex_unlock((pthread_mutex_t *)arg);
    }
}
void *thread2(void *arg)
{
    while(1)
    {
        pthread_mutex_lock((pthread_mutex_t *)arg);
        printer("world");
        pthread_mutex_unlock((pthread_mutex_t *)arg);
    }
}

int main()
{
    //线程互斥锁初始化
    pthread_mutex_t mutex2;
    pthread_mutex_init(&mutex2,NULL);

    //创建线程
    pthread_t id1,id2;
    pthread_create(&id1,NULL,thread1,(void*)&mutex2);
    pthread_create(&id2,NULL,thread2,(void*)&mutex2);

    //等待线程
    pthread_join(id1,NULL);
    pthread_join(id2,NULL);

    //销毁互斥锁
    pthread_mutex_destroy(&mutex2);
}
#else

void *thread(void *arg)
{
    pthread_mutex_lock(&mutex);
	printer((char*)arg);
    pthread_mutex_unlock(&mutex);
}

int main()
{

    //创建线程
    pthread_t id1,id2;
    pthread_create(&id1,NULL,thread,(char*)"hello");
    pthread_create(&id2,NULL,thread,(void*)"world");

    //等待线程
    pthread_join(id1,NULL);
    pthread_join(id2,NULL);

    
}

#endif