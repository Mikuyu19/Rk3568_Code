#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// 信号量联合体
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int main()
{
    // 1. 创建IPC key
    key_t key = ftok(".", 'S');
    if (key == -1)
    {
        perror("ftok失败");
        return -1;
    }

    // 2. 创建信号量集合(5个信号量)
    int semid = semget(key, 5, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget失败");
        return -1;
    }

    printf("成功创建信号量集合,ID: %d\n", semid);

    // 3. 初始化所有信号量的值
    union semun arg;
    unsigned short values[5] = {1, 2, 3, 4, 5};  // 分别设置为1,2,3,4,5
    arg.array = values;
    
    if (semctl(semid, 0, SETALL, arg) == -1)
    {
        perror("semctl初始化失败");
        return -1;
    }

    printf("已初始化5个信号量,值分别为: 1, 2, 3, 4, 5\n");

    // 4. 获取第三个信号量的值(索引为2)
    int val = semctl(semid, 2, GETVAL);
    if (val == -1)
    {
        perror("获取信号量值失败");
        return -1;
    }

    printf("第三个信号量的值为: %d\n", val);

    // 5. 清理资源
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        perror("删除信号量集合失败");
        return -1;
    }

    printf("信号量集合已删除\n");

    return 0;
}
