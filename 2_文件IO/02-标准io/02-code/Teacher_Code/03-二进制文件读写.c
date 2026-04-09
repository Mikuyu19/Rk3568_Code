#include <stdio.h>
#include <stdlib.h>
int main()
{
    //打开文件
    FILE *fp = fopen("1.txt","w+");

    //写
    int a[10] = {1,2,3,4,5,6,7,8,9,10};
    int n = fwrite(a,sizeof(a[0]),10,fp);
    printf("实际写入了%d个元素\n",n);

    //定位光标到文件开头
    fseek(fp,0,SEEK_SET);//rewind(fp);

    //读
    int *p = malloc(sizeof(int)*n);
    n = fread(p,sizeof(p[0]),n,fp);

    //打印读取的数据
    for(int i = 0;i<n;i++)
    {
        printf("%d ",p[i]);
    }
    printf("\n");

    //关闭文件
    fclose(fp);
}