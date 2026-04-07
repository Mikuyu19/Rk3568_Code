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
返回值：
	成功	返回一个被打开的文件流 FLIE*
	失败	NULL error被设置

#include <stdio.h>
int fclose(FILE *stream);
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
```

练习：利用fgetc和fputc实现从键盘输入字符，再显示到标准输出中去，直到遇到‘#’才结束

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
```

```c
#include <stdio.h>
//将s指向的字符串，输出到stream流
int puts(const char *s);//默认输出到终端 stdout
int fputs(const char *s, FILE *stream);
返回值：
	成功 返回非负整数
	失败 -1，errno被设置	
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

(3)二进制文件的读写

```c
#include <stdio.h>
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
ptr:指向的内存空间，用来保存从文件流中读取到的数据的 "数组"
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
之后检查以下文件的大小和你写入的大小是否一致(ls -l stu_info.txt)
接着把stu_info.txt中的数据进行读取，打印出来
获取最高分的学生信息
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
```

# 4.冲刷流

```
#include <stdio.h>

int fflush(FILE *stream);
stream：要同步/更新的文件流
返回值：
	成功 0
	失败 -1 errno
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

