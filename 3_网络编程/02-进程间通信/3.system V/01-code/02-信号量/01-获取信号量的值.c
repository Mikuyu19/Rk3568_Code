#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
int main()
{
	//ftok获取许可证
	key_t key = ftok("/home/china",1);
	if(key == -1)
	{
		perror("ftok failed");
        return -1;
	}   
    
    //创建或打开信号量集合
    int sem_id = semget(key,5,IPC_CREAT | IPC_EXCL | 0777);
	if(sem_id == -1)
	{
		//如果是因为信号量集合存在而失败，则直接打开
		if(errno == EEXIST)
		{
			sem_id = semget(key,5,0);//如果是打开会自动忽略第二个参数
		}
		else 
		{
			perror("semget failed");
			return -1;
		}
	}

    //设置初值
    unsigned short array[5] = {3,4, 5 ,0 ,0};
    semctl(sem_id,0,SETALL,array);  
    
    //获取第三个信号量的值
    int val = semctl(sem_id,2,GETVAL);
    printf("第三个信号量的值为%d\n",val);

    // //删除信号量集合
    // semctl(sem_id,0,IPC_RMID);
}