# posix信号量

有名信号量

无名信号量

a.创建或打开一个posix信号量

>   (1)sem_open创建或初始化一个posix有名信号量
>
>   ```c
>   #include <fcntl.h>           /* For O_* constants */
>   #include <sys/stat.h>        /* For mode constants */
>   #include <semaphore.h>
>   
>   sem_t *sem_open(const char *name, int oflag);
>   sem_t *sem_open(const char *name, int oflag,
>   mode_t mode, unsigned int value);
>   @name:指定要创建或打开的posix信号量在文件系统中的路径名，一定要以"/"开头
>   	eg:"/home" "/data" "/dev" 存在与否都可以
>   @oflag
>   	(1)创建 O_CREAT
>   		判断文件是否存在
>   		O_CREAT | O_EXCL
>   	(2)打开 0
>   第三个参数和第四个参数  当第二个参数是 创建一个有名信号量的时候，才需要
>   @mode：权限
>   	1.宏
>   	2.八进制 0777
>   @value：指定信号量的值
>   返回值：
>   	成功返回sem_t类型的指针，指向一个已经打开的posix信号量
>   	失败 返回SEM_FAILED
>   Link with -pthread.
>   编译需要加 -pthread
>   gcc xxx.c -l pthread
>   
>   eg:
>   	sem_t*sem = sem_open("/home",O_CREAT | O_EXCL,0777,1);
>   	if(sem == SEM_FAILED)
>   	{
>   		if(errno == EEXIST)//存在
>   		{
>   			//打开的方式
>   			sem = sem_open("/home",0);
>   		}
>   		else
>   		{
>   			perror("sem_open failed");
>   			return -1;
>   		}
>   	}
>   ```
>
>   (2)sem_init初始化一个posix无名信号量
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_init(sem_t *sem, int pshared, unsigned int value);
>   @sem：要初始化的posix无名信号量的首地址
>   	1.sem_t sem;//传参数&sem
>   	2.sem_t *sem=malloc(sizeof(sem_t));//传参数sem
>   @pshared:指定无名信号量的共享方式
>   	0：进程内部的线程共享，sem指向的是进程内部的共享区域
>   	1(非0)：不同进程间的共享，sem指向的内核共享的区域 <---------
>   @value:设置信号量的初值
>   返回值：
>   	成功 0
>   	失败 -1
>   Link with -pthread.
>   编译需要加 -pthread
>   
>   eg:
>   	sem_t sem;
>   	int ret = sem_init(&sem,1,1);
>   	if(ret == -1)
>   	{
>   	
>   	}
>   ```
>
>   

b.posix信号量P/V操作

>   1.p操作sem_wait
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_wait(sem_t *sem);//阻塞函数  上锁p
>   返回值：
>   	成功0
>   	失败-1
>   	
>   int sem_trywait(sem_t *sem);//非阻塞
>   返回值：
>   	成功0
>   	失败-1
>   	
>   int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);//限时等待
>   @abs_timeout:获取当前的时间(1970.1.1经过的秒数)+愿意等待的时间
>   
>   Link with -pthread.
>   ==================================
>   #include <time.h>
>   
>   
>   int clock_gettime(clockid_t clk_id, struct timespec *tp);
>   功能：获取时间(1970.1.1)
>   @clk_id：CLOCK_REALTIME
>   @tp:保存获取的时间
>   Link with -lrt 
>   
>   eg:
>   	愿意等待5.2s
>   	//获取当前的时间
>   	struct timespec tp;
>   	clock_gettime(CLOCK_REALTIME,&tp);
>   	tp.tv_sec+=5;//秒
>   	tp.tv_nsec+=200000000;//纳秒
>   	if(tp.tv_nsec >= 1000000000)//进位
>   	{
>   		tp.tv_sec+=1;
>   		tp.tv_nsec-=1000000000
>   	}
>   	sem_timedwait(sem, &tp);//限时等待5.2s
>   ```
>
>   2.v操作 sem_post
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_post(sem_t *sem);
>   
>   Link with -pthread.
>   ```
>
>   

c.posix信号量其他操作

>   1.用来获取信号量的值
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_getvalue(sem_t *sem, int *sval);
>   
>   Link with -pthread.
>   eg:
>   	int val;
>   	sem_getvalue(sem, &val);//val就是你信号量当前的值
>   ```
>
>   2.posix有名信号量的关闭和删除
>
>   ```c
>   #include <semaphore.h>
>   int sem_close(sem_t *sem);
>   Link with -pthread.
>   ```
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_unlink(const char *name);
>   //name要填有名信号量的路径
>   Link with -pthread.
>   ```
>
>   3.无名信号量的删除
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_destroy(sem_t *sem);
>   Link with -pthread.
>   ```
>
>   

有名信号量

```
1.sem_open
2.pv操作(sem_wait  sem_post)
3.sem_close(关闭信号量)
4.sem_unlink(删除信号量)
```

无名信号量

```
1.sem_init
2.pv操作(sem_wait  sem_post)
3.sem_destroy(删除信号量)
```

练习：在昨天共享内存20w的题目中，请分别使用有名信号量和无名信号量分别写一份代码