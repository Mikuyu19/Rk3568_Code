#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    #if 0
	printf("abc");
	fork();
    #else
	printf("abc\n");
	fork();
    #endif
}