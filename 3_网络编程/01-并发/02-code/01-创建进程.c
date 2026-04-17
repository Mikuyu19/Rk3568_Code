#include <sys/types.h>
#include <unistd.h>

int main()
{
    fork();
	fork();
	printf("hello\n");
}