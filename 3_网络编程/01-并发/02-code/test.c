#include <stdio.h>
#include <unistd.h>
int main()
{
    char *argv[] = {"./08-test", "3", "4", NULL};
    int ret = execvp("/mnt/c/Users/Zelda/Desktop/Rk3568_Code/3_网络编程/01-并发/02-code/08-test", argv);
    if (ret == -1)
    {
        perror("Error");
        return -1;
    }
    printf("ret = %d\n", ret);
    return 0;
}
