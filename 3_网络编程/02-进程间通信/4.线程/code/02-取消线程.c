#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
void *abc(void*arg)
{
    //设置不能被取消
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
	while(1)
	{
		printf("hello\n");

		sleep(1);
	}
    pthread_exit((void*)-1);
}
int main()
{
	//创建一个线程
	pthread_t thread;
	pthread_create(&thread,NULL,abc,NULL);
    int i = 0;
	while(1)
	{
		printf("world\n");
        i++;
        if(i == 5)
        {
            //取消线程
            pthread_cancel(thread);
        }
		sleep(1);
	}

    
	
}