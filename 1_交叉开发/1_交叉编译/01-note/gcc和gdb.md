# 1.gcc的步骤

```
gcc main.c a.c b.c -o main
```

编译的步骤

```c
1.预处理
	gcc -E main.c -o main.i
	=>main.i就是预处理后的文件
```

```
2.编译
	把C程序 编译成 汇编代码
	gcc -S main.i -o main.s
	=>main.s就是汇编代码
```

```
3.汇编
	把汇编程序编译成一个目标文件包(机器指令文件)
	gcc -c main.s -o main.o
```

```
4.链接
	把各个目标文件.o以及库文件链接成一个可执行文件
	gcc main.o a.o b.o -o a.out
	=>a.out可执行文件
```

# 2.gdb

```
单步调试工具

下载：
	sudo apt install gdb

1.gcc -g main.c -o main
	//-g 加入gdb的调试信息
2.调试程序
	gdb 可执行程序
	eg:
		gdb main
```

```
gdb调试的命令

b
	breakpoint 用于设置断点

info b
	查看所有的断点信息
	
d
	删除断点 delete

r
	run运行代码，直到到断点出或程序结束

n 
	next下一步
	单步调试
	对于函数调用 看成是一步 直接函数执行完
s
	next下一步
	单步运行
	对于函数 进入到函数内部
	finish 用来结束当前该函数
	
quit
	退出
	
print 
	打印变量
	print i
	print/x i 十六进制
	print/x &i	打印i的地址
```

```
经常用来找段错误
```

请使用gdb找出下列代码出现段错误的位置

```c
#include <stdio.h>
void swap(int *x,int *y)
{
	int *temp;
	*temp = *x;
	*x = *y;
	*y = *temp;
}
int main()
{
	int a=3,b=4;
	swap(&a,&y);
}
```

