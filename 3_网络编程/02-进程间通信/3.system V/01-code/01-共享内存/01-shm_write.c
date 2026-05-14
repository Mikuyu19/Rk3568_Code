#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
int main()
{
	//ftok获取许可证
	key_t key = ftok("/home/china",1);
	if(key == -1)
	{
		perror("ftok failed");
        return -1;
	}
	//shmget打开或创建共享内存
	int shm_id = shmget(key,4096,IPC_CREAT | IPC_EXCL | 0777);
	if(shm_id == -1)
	{
		//如果是因为共享内存存在而失败，则直接打开
		if(errno == EEXIST)
		{
			shm_id = shmget(key,0,0);
		}
		else 
		{
			perror("shmget failed");
			return -1;
		}
	}
	//shmat映射 建立你的指针和共享内存的关系
    char *p = shmat(shm_id,NULL,0);
	if((void*)p == (void*)-1)//失败
	{
		perror("shmat failed");
        return -1;
	}    
	//写数据
    strcpy(p,"nihao");
	//shmdt断开映射关系   
    shmdt(p);	
}