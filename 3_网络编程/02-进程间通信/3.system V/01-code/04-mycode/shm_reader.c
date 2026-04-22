#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main()
{
    key_t key = ftok(".", 'A');
    int shmid = shmget(key, 1024, 0666);
    if (shmid == -1)
    {
        perror("shmget 失败");
        return -1;
    }
    // 3. 映射
    char *str = (char *)shmat(shmid, NULL, 0);
    printf("内容是: %s\n", str);
    // 5. 解除映射
    shmdt(str);
    // 6. 销毁共享内存！
    if (shmctl(shmid, IPC_RMID, NULL) == 0)
    {
        printf("已销毁共享内存\n");
    }

    return 0;
}