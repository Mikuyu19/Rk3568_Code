#include <stdio.h>
#include <string.h>
int main(void)
{
    int age;
    char name[100];
    printf("请输入年龄：");
    scanf("%d", &age);
    getchar();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("age = %d\n", age);
    printf("name = [%s]\n", name);
    return 0;
}
