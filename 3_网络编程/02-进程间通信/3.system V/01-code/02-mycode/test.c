#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <unistd.h>

// 加锁
void P(int semid)
{
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

// V操作（解锁）
void V(int semid)
{
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main()
{
    key_t key = ftok(".", 'A');
    if (key == -1)
    {
        perror("ftok 失败");
        return -1;
    }
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget 失败");
        return -1;
    }

    int *p = (int *)shmat(shmid, NULL, 0);
    if (p == (void *)-1)
    {
        perror("shmat 失败");
        return -1;
    }
    *p = 0;

    // 创建信号量
    int semid = semget(key, 1, IPC_CREAT | 0666);
    union semun
    {
        int val;
    } arg;
    arg.val = 1; // 初值=1，用作互斥锁
    semctl(semid, 0, SETVAL, arg);

    pid_t pid = fork();

    if (pid > 0) // 父进程
    {
        // 执行10万次
        for (int i = 0; i < 100000; i++)
        {
            P(semid);
            (*p)++;
            V(semid);
        }
        wait(NULL);
        printf("最终结果：%d\n", *p);
        // 解除映射
        shmdt(p);
        // 删除共享内存
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID); // 删除信号量
    }
    else if (pid == 0) // 子进程
    {
        // 执行10万次
        for (int i = 0; i < 100000; i++)
        {
            P(semid);
            (*p)++;
            V(semid);
        }
        shmdt(p);
    }
    else
    {
        perror("fork 失败");
        return -1;
    }

    return 0;
}
