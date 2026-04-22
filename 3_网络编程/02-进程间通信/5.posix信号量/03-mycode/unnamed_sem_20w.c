#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

// 共享内存结构体
typedef struct
{
    sem_t sem;  // 无名信号量
    int count;  // 计数器
} shared_data_t;

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
    int shm_id = shmget(key, sizeof(shared_data_t), IPC_CREAT | IPC_EXCL | 0777);
    if (shm_id == -1)
    {
        if (errno == EEXIST)
        {
            shm_id = shmget(key, sizeof(shared_data_t), 0);
        }
        else
        {
            perror("shmget failed");
            return -1;
        }
    }

    // 3. 映射共享内存
    shared_data_t *shm = (shared_data_t *)shmat(shm_id, NULL, 0);
    if ((void *)shm == (void *)-1)
    {
        perror("shmat failed");
        return -1;
    }

    // 4. 初始化无名信号量（进程间共享，初值为1）
    if (sem_init(&shm->sem, 1, 1) == -1)
    {
        perror("sem_init failed");
        return -1;
    }

    shm->count = 0; // 初始化计数器

    // 5. 创建子进程
    pid_t pid = fork();
    if (pid > 0) // 父进程
    {
        int a = 0;
        while (a < 100000)
        {
            sem_wait(&shm->sem);  // P操作，加锁
            shm->count++;
            sem_post(&shm->sem);  // V操作，解锁
            a++;
        }

        wait(NULL); // 等待子进程结束
        printf("最终结果: %d\n", shm->count);

        // 清理资源
        sem_destroy(&shm->sem);
        shmdt(shm);
        shmctl(shm_id, IPC_RMID, NULL);
    }
    else if (pid == 0) // 子进程
    {
        int a = 0;
        while (a < 100000)
        {
            sem_wait(&shm->sem);  // P操作，加锁
            shm->count++;
            sem_post(&shm->sem);  // V操作，解锁
            a++;
        }

        // 清理资源
        shmdt(shm);
    }

    return 0;
}
