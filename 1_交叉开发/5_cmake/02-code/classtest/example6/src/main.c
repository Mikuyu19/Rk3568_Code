#include <stdio.h>
#include "add.h"
#include "sub.h"

int main()
{
	printf("3 + 4 = %d\n",add(3,4));
    printf("3 - 4 = %d\n",sub(3,4));
	return 0;
}