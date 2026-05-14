#include <stdio.h>
#include <stdlib.h>

typedef struct stu
{
	int id;
	int score;
}stu;

int main()
{
    //打开文件
    FILE *fp = fopen("stu_info.txt","w+");

    //输入学生人数
    int n;
    scanf("%d",&n);

    //从键盘输入每个学生的信息
    stu *s = malloc(sizeof(stu)*n);//开辟空间存储学生的信息
    for(int i = 0;i<n;i++)
        scanf("%d%d",&s[i].id,&s[i].score);

    //写
    int m = fwrite(s,sizeof(s[0]),n,fp);
    printf("实际写入了%d个元素\n",m);

    //定位光标到文件开头
    fseek(fp,0,SEEK_SET);//rewind(fp);

    //释放空间
    free(s);

    //读
    stu *p = malloc(sizeof(stu)*m);
    int q = fread(p,sizeof(p[0]),m,fp);

    //打印读取的数据
    int max_index = 0;
    for(int i = 0;i<q;i++)
    {
        if(p[max_index].score < p[i].score)
            max_index = i;
        printf("%d %d\n",p[i].id,p[i].score);
    }

    printf("最高分的学生id为%d 分数为%d\n",p[max_index].id,p[max_index].score);

    //释放空间
    free(p);
    
    //关闭文件
    fclose(fp);
}