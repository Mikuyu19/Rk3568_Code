统一文件的接口(不同的操作系统中都使用标准IO)

系统IO只能在当前的系统中使用

标准IO的特点

```
1.是在系统IO的基础上实现的(调用标准IO，底层实际上调用的系统IO)
2.标准IO是带缓冲的IO，系统IO不带缓冲的(缓冲可以理解成一段内存)
3.标准IO的效率会比系统IO高
	标准IO
		读取一个字节，通过一次系统调用从硬盘中读取一块(512B)出来,放在标准IO的缓冲区
	系统IO
		读取一个字节，从硬盘中读取一个字节
4.标准IO只能访问"文件"(文本文件 二进制文件)
	系统io可以访问文件(linux下一切皆文件，包含设备)
```

```
FILE  描述一个打开的文件
FILE
{
	char *in;//指向读缓冲区
	char *out;//指向写缓冲区
}

int *p = NULL;
printf("abcd");
printf("%d\n",*p);
```

标准IO操作文件的过程

```
标准IO的函数->标准IO库->系统IO函数->linux操作系统->硬件
```

流stream

```
流是一串连续不断的输出数据集合，就像水管里面的水流，在水管的一端供水，另外一端就看到连续不断地水流
```

同步

```c
标准IO的缓冲区分成三种类型
1.行缓冲
	缓冲区的数据达到了一行(系统设定)或遇到了'\n',才会同步到外设中
	
	假设缓冲区一行最多是80个字节
	printf是行缓冲
	eg:
		printf("abcd");
		while(1);
	eg:
		printf("abcd\n");
		while(1);	
2.全缓冲
	缓冲区的数据要填满整个缓冲区的大小，才会同步到外设中
		4k 8k
3.无缓冲
	缓冲区只要有一个字节，才会同步到外设中
	perror
```

标准io会自动打开3个流

```
标准输入流  	stdin
标准输出流	stdout
标准错误流	stderr
```

# 1.打开和关闭函数

```c
#include <stdio.h>

FILE *fopen(const char *pathname, const char *mode);
pathname:要打开的路径
mode:打开的方式 “字符串” 
	r	只读打开,文件不存在，则会报错，打开之后，光标在文件开头
	r+	读写打开,文件不存在，则会报错，打开之后，光标在文件开头
	w	只写打开,文件不存在，则创建，
				文件存在,文件的内容会被清空
	w+	读写打开,文件不存在，则创建，
				文件存在,文件的内容会被清空
	a	只写打开，追加的方式打开，文件不存在，则创建
				打开后，光标在文件的末尾
	a+	读写打开,追加的方式打开，文件不存在，则创建
				打开后，写光标在文件的末尾
				打开后，读光标在文件的开头
              ┌─────────────┬───────────────────────────────┐
              │fopen() mode │ open() flags                  │
              ├─────────────┼───────────────────────────────┤
              │     r       │ O_RDONLY                      │
              ├─────────────┼───────────────────────────────┤
              │     w       │ O_WRONLY | O_CREAT | O_TRUNC  │
              ├─────────────┼───────────────────────────────┤
              │     a       │ O_WRONLY | O_CREAT | O_APPEND │
              ├─────────────┼───────────────────────────────┤
              │     r+      │ O_RDWR                        │
              ├─────────────┼───────────────────────────────┤
              │     w+      │ O_RDWR | O_CREAT | O_TRUNC    │
              ├─────────────┼───────────────────────────────┤
              │     a+      │ O_RDWR | O_CREAT | O_APPEND   │
              └─────────────┴───────────────────────────────┘

返回值：
	成功	返回一个被打开的文件流 FLIE*
	失败	NULL error被设置

#include <stdio.h>
int fclose(FILE *stream);
```

eg:打开一个1.txt文件，以只读的方式打开

```c
FILE*fp =  fopen("1.txt","r");
if(fp == NULL)
{
	perror("fopen fail");
}
```

# 2.读写流

(1)每次读写一个字符

```c
#include <stdio.h>
用来从stream指定的文件流中读取一个字符
int fgetc(FILE *stream);//函数
int getc(FILE *stream);//宏
stream：指定从哪个文件中读取字符
	一般stream要从fopen获取
	如果从键盘获取字符 stdin
int getchar(void);
返回值：
	成功 返回读到的那个字符的ascii(>=0)
	失败 -1 errno被设置
eg:
	getchar() == fgetc(stdin)  等价
	char c = getchar();//c里面保存的就是刚才从键盘获取的字符
```

```c
#include <stdio.h>

int fputc(int c, FILE *stream);
int putc(int c, FILE *stream);
int putchar(int c);
c:要写入的字符ascii
stream:指定要写入的文件流
返回值
	成功 返回写入的那个字符的ascii(>=0)
	失败 -1 errno被设置	
	
	char c = 'A';
	putchar(c)	//要打印输出变量c  打印出的结果为A字符
        
    putchar(c) == fputc(c, stdout)  等价
```

练习：利用fgetc和fputc实现从键盘输入字符，再显示到标准输出中去，直到遇到‘#’才结束

```c
#include <stdio.h>

int main()
{
#if 0
    while(1)
    {
        //从键盘获取字符
        char c = fgetc(stdin);
        //判断获取的字符是否为# 是则跳出循环 否执行后面代码
        if(c == '#')
            break;

        //打印输出
        fputc(c, stdout);
    }
#else
    char c;
    while((c = fgetc(stdin))!='#')
    {
        //打印输出
        fputc(c, stdout);
    }
#endif
}
```

(2)每次一行读写

```c
#include <stdio.h>
char *gets(char *s);//默认从键盘获取输入 stdin
char *fgets(char *s, int size, FILE *stream);
s:指向保存获取的字符串的空间
size:指向s指向的空间的字节数(最多能存储size个字节)
stream:获取用字符串的那个文件流
返回只：
	成功 返回s指向的空间的地址
	失败 NULL，errno被设置
eg:
	char s[100] = {0};//用来保存从键盘获取的字符串
	gets(s);//从键盘获取字符串保存到s指向的空间中
eg:
	char s[100] = {0};//用来保存从键盘获取的字符串
	fgets(s,sizeof(s)-1,stdin);//从标准输入中最多获取sizeof(s)-1个字节，保存到s指向的空间中

gets(s) == fgets(s,sizeof(s)-1,stdin) 等价
需要注意fget会获取回车
假设输入abc回车，获取的结果abc\n\0
怎么把结果中的回车去掉
char s[]="abc\n\0";
s[strlen(s)-1] = '\0';
```

```c
#include <stdio.h>
//将s指向的字符串，输出到stream流
int puts(const char *s);//默认输出到终端 stdout
int fputs(const char *s, FILE *stream);
返回值：
	成功 返回非负整数
	失败 -1，errno被设置	

eg:
	puts("abcd");
	char *s = "abcd";
	puts(s);
	char s2[100] = "abcd";
	puts(s2);
eg：
	char *s = "abcd";
	fputs(s, stdout);//将s指向的空间的字符串输出到stdout的那个文件中去


注意：puts(s) 会加换行
	puts(s) == fputs(s, stdout);//没有换行	
				fputs("\n", stdout);	
```

文件结束的判断

```
#include <stdio.h>

int feof(FILE *stream);
功能：用于判断一个文件是否已经结束
返回值：
	真	文件已经到末尾了
	假	文件还没有读到末尾
```

练习：用标准io实现cp(拷贝一个文件的内容)

```c
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
```

(3)二进制文件的读写

```c
#include <stdio.h>
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
ptr:指向的内存空间，用来保存从文件流中读取到的数据  "数组"
size:每个元素占的字节数
nmemb:要读取的元素的个数
返回值：
	返回实际读到的元素的个数
```

```
#include <stdio.h>
size_t fwrite(const void *ptr, size_t size, size_t nmemb,FILE *stream);
ptr:指向的内存空间，用来放写入的内容的首地址 "数组"
size:每个元素占的字节数
nmemb:要写入的元素的个数
返回值：
	返回实际写入的元素的个数
```

eg：把int a[10]的数据写入到1.txt文件，在把写入的数据读取出来

```c
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
```

练习：

```
有一个学生信息的结构体
typedef struct stu
{
	int id;
	int score;
}stu;

从键盘输入每个学生的信息，然后通过标准io库的函数
把每个学生的信息写入到stu_info.txt
之后检查一下文件的大小和你写入的大小是否一致(ls -l stu_info.txt)
接着把stu_info.txt中的数据进行读取，打印出来
获取最高分的学生信息
```

```c
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
```

# 3.定位流

```c
#include <stdio.h>

int fseek(FILE *stream, long offset, int whence);
返回值：
	0 成功
	-1 失败

long ftell(FILE *stream);
//获取当前光标的位置距离文件开头的字节数

void rewind(FILE *stream);
//把文件的光标定位到文件开头

//把文件的光标定位到文件开头
rewind(fp) == fseek(fp,0,SEEK_SET) 
//获取文件的大小
ftell(fp) 
lseek(fd,0,SEEK_END)
```

# 4.冲刷流

```
#include <stdio.h>

int fflush(FILE *stream);
stream：要同步/更新的文件流
返回值：
	成功 0
	失败 -1 errno
	
printf("abc");
fflush(stdout);//把缓冲区的内容直接同步到终端，就相当于输出到终端
while(1);
```

# 5.格式化io

1.格式化输入

​	按照指定的格式进行输入

```
#include <stdio.h>
int scanf(const char *format, ...);
scanf可以带很多的参数的，它的参数分成两类
第一个参数为一类，格式化字符串，FORMAT STRING
其它的参数为另一类,地址列表
	格式化字符串中的转义字符就会对应一个地址，把转义字符所对应的输入存储到后面的地址中去
返回值：
	匹配成功的变量的个数
```

```c
int fscanf(FILE *stream, const char *format, ...);
从stream中获取数据
跟scanf类似的
```

```c
int sscanf(const char *str, const char *format, ...);
从str字符串获取数据
跟scanf类似的
```

练习：

```c
1.请问下列代码片段中最后a b c的值为多少
	char *str = "123BCDDDDDD";
	int a;
	char c;
	int b;
	b = sscanf(str,"%d %c",&a,&c);
2.请把下列的数字字符串调用函数转成数字
	char *s="123";
```

2.格式化输出

​	按照我们指定的格式进行输出

```c
#include <stdio.h>

int printf(const char *format, ...);
第一个参数：format告诉程序怎么输出
其它参数：
	要输出的变量
返回值：
	实际打印的字符的个数
	
int fprintf(FILE *stream, const char *format, ...);
//输出的地方是stream代表的那个文件
参数和返回值跟printf是一样的

int dprintf(int fd, const char *format, ...);
//输出的地方是fd代表的那个文件(一般用open获取)
int sprintf(char *str, const char *format, ...);
//输出的地方是str的字符串
int snprintf(char *str, size_t size, const char *format, ...);
//输出的地方是str的字符串
size:指定最大的空间的大小，格式化输出的最大的输出size-1个字节
返回值：
	输出的字符串的长度，而不是实际存储到str中的长度
```

练习：

```c
新建一个4.txt
id:1 name:zhangsan score:76
id:2 name:wangwu score:80
id:3 name:lisi score:66
typedef struct stu
{
	int id;
	char name[54];
	int score;
}stu;
使用fscanf从4.txt中去获取全部学生信息，打印输出
通过fprintf将学生的所有信息输出到stu2.txt中去
```

