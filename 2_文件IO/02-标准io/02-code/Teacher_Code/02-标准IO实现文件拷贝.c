#include <stdio.h>

int main()
{
    //打开两个文件
    FILE *read_fp = fopen("read.txt","r");
    FILE *write_fp = fopen("write.txt","w");

    
    while(1)
    {
        //判断文件是否结束
        if(feof(read_fp))
            break;
        //读取文件内容fgets
        char s[100] = {0};//用来保存从文件中获取的字符串
        fgets(s,sizeof(s)-1,read_fp);//从文件中最多获取sizeof(s)-1个字节，保存到s指向的空间中
        
        //把刚才读到的东西写入到文件fputs
        fputs(s, write_fp);
    }

    //关闭两个文件  
    fclose(read_fp);
    fclose(write_fp);
}