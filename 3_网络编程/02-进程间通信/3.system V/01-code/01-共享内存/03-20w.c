#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
int main()
{
    // ftok获取许可证
    key_t key = ftok(".", "A");
    if (key == -1)
    {
        perror("ftok failed");
        return -1;
    }
    // shmget打开或创建共享内存
    int shm_id = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0777);
    if (shm_id == -1)
    {
        // 如果是因为共享内存存在而失败，则直接打开
        if (errno == EEXIST)
        {
            shm_id = shmget(key, 0, 0);
        }
        else
        {
            perror("shmget failed");
            return -1;
        }
    }
    // shmat映射 建立你的指针和共享内存的关系
    int *p = shmat(shm_id, NULL, 0);
    if ((void *)p == (void *)-1) // 失败
    {
        perror("shmat failed");
        return -1;
    }
    *p = 0; // 初始化
    pid_t pid = fork();
    if (pid > 0)
    {
        int a = 0;
        while (a < 100000)
        {
            // 执行10万次
            (*p)++;
            a++;
        }
        wait(NULL); // wait子进程结束
        printf("%d\n", *p);

        // shmdt断开映射关系
        shmdt(p);

        // 删除共享内存
        shmctl(shm_id, IPC_RMID, NULL);
    }
    else if (pid == 0)
    {
        int a = 0;
        while (a < 100000)
        {
            // 执行10万次
            (*p)++;
            a++;
        }
        // shmdt断开映射关系
        shmdt(p);
    }
}