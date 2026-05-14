// 新建一个4.txt
//     id : 1 name : zhangsan score : 76 id : 2 name : wangwu score : 80 id : 3 name : lisi score : 66 typedef struct stu
// {
//     int id;
//     char name[54];
//     int score;
// } stu;
// 使用fscanf从4.txt中去获取全部学生信息，打印输出
//     通过fprintf将学生的所有信息输出到stu2.txt中去

#include <stdio.h>
#include <stdlib.h>

typedef struct stu
{
    int id;
    char name[54];
    int score;
} stu;

int main(void)
{
    FILE *src = fopen("4.txt", "r");
    FILE *dst = NULL;
    stu s;
    if (src == NULL)
    {
        perror("failed open src");
        return 1;
    }

    dst = fopen("stu2.txt", "w");
    if (dst == NULL)
    {
        perror("failed open dst");
        return 1;
    }

    // 从4.txt中读取数据
    while (fscanf(src, " id:%d name:%s score:%d", &s.id, s.name, &s.score) == 3)
    {
        printf("id:%d name:%s score:%d\n", s.id, s.name, s.score);
        fprintf(dst, "id:%d name:%s score:%d\n", s.id, s.name, s.score);
    }
    fclose(src);
    fclose(dst);
    printf("print ok\n");
    return 0;
}
