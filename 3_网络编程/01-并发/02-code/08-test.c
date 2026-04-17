#include <stdio.h>
#include <stdlib.h>

//gcc 08-test.c -o 08test
//./08test 3 4
int main(int argc,char **argv)
{
    printf("%s + %s = %d\n",argv[1],argv[2],atoi(argv[1])+atoi(argv[2]));
}