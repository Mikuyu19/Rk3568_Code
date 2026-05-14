#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    // 1. ftok获取许可证
    key_t key = ftok(".", 'A');
    if (key == -1)
    {
        perror("ftok failed");
        return -1;
    }

    // 2. 创建共享内存
    int shm_id = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0777);
    if (shm_id == -1)
    {
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

    // 3. 映射共享内存
    int *p = shmat(shm_id, NULL, 0);
    if ((void *)p == (void *)-1)
    {
        perror("shmat failed");
        return -1;
    }

    *p = 0; // 初始化为0

    // 4. 创建有名信号量（互斥锁，初值为1）
    sem_t *sem = sem_open("/sem_20w", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open failed");
        return -1;
    }

    // 5. 创建子进程
    pid_t pid = fork();
    if (pid > 0) // 父进程
    {
        int a = 0;
        while (a < 100000)
        {
            sem_wait(sem);  // P操作，加锁
            (*p)++;
            sem_post(sem);  // V操作，解锁
            a++;
        }

        wait(NULL); // 等待子进程结束
        printf("结果: %d\n", *p);

        // 清理资源
        shmdt(p);
        sem_close(sem);
        sem_unlink("/sem_20w");
        shmctl(shm_id, IPC_RMID, NULL);
    }
    else if (pid == 0) // 子进程
    {
        int a = 0;
        while (a < 100000)
        {
            sem_wait(sem);  // P操作，加锁
            (*p)++;
            sem_post(sem);  // V操作，解锁
            a++;
        }

        // 清理资源
        shmdt(p);
        sem_close(sem);
    }

    return 0;
}
