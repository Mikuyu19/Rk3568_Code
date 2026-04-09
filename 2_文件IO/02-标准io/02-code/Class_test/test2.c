// 练习：用标准io实现cp(拷贝一个文件的内容)
#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *fp1 = fopen("1.txt", "r"); // 打开源文件
    if (fp1 == NULL)
    {
        perror("Failed to open file1");
        return 1;
    }
    FILE *fp2 = fopen("2.txt", "w"); // 打开目标文件
    if (fp2 == NULL)
    {
        perror("Failed to open file2");
        fclose(fp1);
        return 1;
    }
    char ch;
    while ((ch = fgetc(fp1)) != EOF) // 从源文件读取字符，直到文件末尾
    {
        fputc(ch, fp2); // 将读取的字符写入目标文件
    }
    // 关闭文件
    fclose(fp1);
    fclose(fp2);
    // 打印完成提示
    printf("File copy OK\n");
    return 0;
}
