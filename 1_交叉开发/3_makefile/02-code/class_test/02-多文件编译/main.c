#include <stdio.h>
#include "add.h"
#include "sub.h"
int main()
{
	printf("3 + 4 = %d\n", add(3, 4));
	printf("10 - 4 = %d\n", sub(10, 4));
	return 0;
}