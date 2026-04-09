// 有一个学生信息的结构体
// typedef struct stu
// {
//     int id;
//     int score;
// } stu;

// 从键盘输入每个学生的信息，然后通过标准io库的函数
//     把每个学生的信息写入到stu_info.txt
//     之后检查以下文件的大小和你写入的大小是否一致(ls - l stu_info.txt)
//         接着把stu_info.txt中的数据进行读取，打印出来
//     获取最高分的学生信息

#include <stdio.h>
#include <stdlib.h>

typedef struct stu
{
    int id;
    int score;
} stu;

int main()
{
    FILE *fp = fopen("stu_info.txt", "w+"); // 打开文件，读写模式
    if (fp == NULL)
    {
        perror("Failed open");
        return 1;
    }
    stu students[100]; // 假设最多有100个学生
    int count = 0;     // 学生数量

    // 从键盘输入学生信息
    printf("输入学生信息 (id score)，以-1结束:\n");
    while (1)
    {
        int id, score;
        scanf("%d", &id);
        if (id == -1)
        {
            break; // 输入-1结束
        }
        scanf("%d", &score);
        students[count].id = id;
        students[count].score = score;
        count++;
    }

    // 将学生信息写入文件
    for (int i = 0; i < count; i++)
    {
        fprintf(fp, "%d %d\n", students[i].id, students[i].score);
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END); // 移动到文件末尾
    long file_size = ftell(fp); // 获取当前文件位置，即文件大小
    printf("File size: %ld bytes\n", file_size);

    // 读取文件内容并打印
    rewind(fp); // 回到文件开头
    printf("文件内容:\n");
    while (!feof(fp))
    {
        int id, score;
        if (fscanf(fp, "%d %d", &id, &score) == 2)
        {
            printf("ID: %d, Score: %d\n", id, score);
        }
    }

    // 获取最高分的学生信息
    int max_score = -1;
    int max_id = -1;
    for (int i = 0; i < count; i++)
    {
        if (students[i].score > max_score)
        {
            max_score = students[i].score;
            max_id = students[i].id;
        }
    }
    printf("最高分学生 ID: %d, 分数: %d\n", max_id, max_score);

    fclose(fp); // 关闭文件
    return 0;
}
