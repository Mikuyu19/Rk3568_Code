#include <stdio.h>
#include "add.h"
#include "sub.h"
#include "bst.h"

int main()
{
	printf("3 + 4 = %d\n",add(3,4));
    printf("3 - 4 = %d\n",sub(3,4));
	binode *r = create_bst();
	print_order(r);
	return 0;
}