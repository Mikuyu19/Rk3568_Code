#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
// ftok 的原型：
// key_t ftok(const char *pathname, int proj_id);

int main()
{
    key_t my_key = ftok("/mnt/c/Users/Zelda/Desktop/Rk3568_Code/3_网络编程/02-进程间通信/3.共享内存/03-mycode", 'A');

    if (my_key == -1)
    {
        perror("生成 Key 失败 (通常是因为路径不存在)");
    }
    else
    {
        printf("生成的独一无二的 IPC_key 是: 0x%x\n", my_key);
    }
    return 0;
}