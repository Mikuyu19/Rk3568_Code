#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

    char *str = (char *)shmat(shmid, NULL, 0);
    if (str == (void *)-1)
    {
        perror("shmat 失败");
        return -1;
    }
    strcpy(str, "Hello! ");
    sleep(5);

    shmdt(str);

    return 0;
}