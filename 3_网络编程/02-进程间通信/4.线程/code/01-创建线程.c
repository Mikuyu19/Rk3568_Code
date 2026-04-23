#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
void *abc(void*arg)
{
	while(1)
	{
		printf("hello\n");
		sleep(1);
	}
}
int main()
{
	//创建一个线程
	pthread_t thread;
	pthread_create(&thread,NULL,abc,NULL);
	while(1)
	{
		printf("world\n");
		sleep(1);
	}
	
}