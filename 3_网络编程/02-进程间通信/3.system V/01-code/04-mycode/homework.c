#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

int sem_id;
int shm_id;
int flag = 1;
// 信号处理函数
void signal_handler(int signum)
{
    flag = 0;
}

// P操作
void sem_p(int sem_id, int index)
{
    struct sembuf buf;
    buf.sem_num = index;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop(sem_id, &buf, 1);
}

// V操作
void sem_v(int sem_id, int index)
{
    struct sembuf buf;
    buf.sem_num = index;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    semop(sem_id, &buf, 1);
}

int main()
{
    // 1. ftok获取许可证
    key_t key = ftok(".", 1);
    if (key == -1)
    {
        perror("ftok failed");
        return -1;
    }

    // 2. 创建共享内存
    shm_id = shmget(key, 4096, IPC_CREAT | IPC_EXCL | 0777);
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

    // 3. 创建信号量集(2个信号量: 读信号量和写信号量)
    sem_id = semget(key, 2, IPC_CREAT | IPC_EXCL | 0777);
    if (sem_id == -1)
    {
        if (errno == EEXIST)
        {
            sem_id = semget(key, 2, 0);
        }
        else
        {
            perror("semget failed");
            return -1;
        }
    }

    // 4. 初始化信号量: arr[0]=0(读信号量), arr[1]=1(写信号量)
    unsigned short arr[2] = {0, 1};
    semctl(sem_id, 0, SETALL, arr);

    // 5. 映射共享内存
    char *p = shmat(shm_id, NULL, 0);
    if ((void *)p == (void *)-1)
    {
        perror("shmat failed");
        return -1;
    }

    // 6. 注册信号处理函数
    signal(SIGINT, signal_handler);

    // 7. 创建子进程
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) // 子进程 - 写数据
    {
        char *s = "123456789";
        int i = 0;

        while (flag)
        {
            // P操作 - 等待写信号量
            sem_p(sem_id, 1);

            // 写入一个字符到共享内存
            *p = s[i];
            i = (i + 1) % strlen(s); // 循环写入

            // V操作 - 释放读信号量
            sem_v(sem_id, 0);

            usleep(100000); // 延时100ms，便于观察
        }
    }
    else // 父进程 - 读数据
    {
        while (flag)
        {
            // P操作 - 等待读信号量
            sem_p(sem_id, 0);

            // 读取并打印字符
            printf("%c", *p);
            fflush(stdout);

            // V操作 - 释放写信号量
            sem_v(sem_id, 1);
        }

        // 等待子进程结束
        wait(NULL);

        // 解除映射
        shmdt(p);

        // 删除信号量和共享内存
        semctl(sem_id, 0, IPC_RMID);
        shmctl(shm_id, IPC_RMID, NULL);
    }
}
