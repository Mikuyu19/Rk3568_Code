#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>

// 信号量联合体
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// P操作（等待/加锁）
void P(int semid)
{
    struct sembuf op = {0, -1, 0};
    if (semop(semid, &op, 1) == -1)
    {
        perror("P操作失败");
        exit(1);
    }
}

// V操作（释放/解锁）
void V(int semid)
{
    struct sembuf op = {0, 1, 0};
    if (semop(semid, &op, 1) == -1)
    {
        perror("V操作失败");
        exit(1);
    }
}

int main()
{
    // 1. 创建IPC key
    key_t key = ftok(".", 'A');
    if (key == -1)
    {
        perror("ftok失败");
        return -1;
    }

    // 2. 创建共享内存
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget失败");
        return -1;
    }

    // 3. 映射共享内存
    int *counter = (int *)shmat(shmid, NULL, 0);
    if (counter == (void *)-1)
    {
        perror("shmat失败");
        return -1;
    }

    // 4. 创建信号量
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget失败");
        return -1;
    }

    // 5. 初始化信号量为1（互斥锁）
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("semctl初始化失败");
        return -1;
    }

    // 6. 初始化共享内存
    *counter = 0;

    printf("开始并发计数测试...\n");

    // 7. 创建子进程
    pid_t pid = fork();

    if (pid > 0)  // 父进程
    {
        for (int i = 0; i < 100000; i++)
        {
            P(semid);           // 加锁
            (*counter)++;       // 临界区
            V(semid);           // 解锁
        }

        // 等待子进程结束
        wait(NULL);

        // 打印最终结果
        printf("最终结果: %d (期望: 200000)\n", *counter);

        // 清理资源
        shmdt(counter);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);

        printf("测试完成，资源已清理\n");
    }
    else if (pid == 0)  // 子进程
    {
        for (int i = 0; i < 100000; i++)
        {
            P(semid);
            (*counter)++;
            V(semid);
        }

        shmdt(counter);
        exit(0);
    }
    else
    {
        perror("fork失败");
        return -1;
    }

    return 0;
}
