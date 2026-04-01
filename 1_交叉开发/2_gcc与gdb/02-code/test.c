#include <stdio.h>

void swap(int *x, int *y)
{
    int *temp;//错误
	//正确做法
	//int temp;
    *temp = *x;

    *x = *y;
    *y = *temp;
}

int main(void)
{
    int a = 3, b = 4;
    swap(&a, &b);
    printf("a=%d b=%d\n", a, b);
    return 0;
}
