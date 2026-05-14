#  一、并发

## 1、进程

### 0、问题的引入

程序通常是按指定的语法编写的好的，用来知名计算机该如何处理数据的规则，其在计算机科学中 的定义 是

$$
对某一个有限的数据集合所实施的，目的在于解决某一问题的一组有限指定的集合
$$
用一个公式可以很好的对程序的定义

$$
程序 = 数据结构 + 算法
$$
eg:

```
输入两个数据，计算这两个数的和
1.等待用户输入数据a,b
2.计算a+b
3.把数据写回
```

程序的执行方式

(1)顺序执行

```
一个程序完全指向完毕之后，才能执行下一个程序
eg:
	把一个程序分成三个步骤
		输入数据	->	计算数据 -> 写回文件(打印出来)
	/*
		在这个过程中，计算数据的时候，cpu(中央处理器)才会被利用起来，而对于输入数据和写回文件的步骤，cpu是空闲状态
	*/
	cpu利用率低
```

(2)并发执行(时间片轮转 ->分时系统)

```
把一个程序分成三个步骤
	输入数据	->	计算数据 -> 写回文件(打印出来)
不同的步骤，由不同的硬件完成，这样就能同时运行多个程序
	硬件1		硬件2		硬件3
1. 输入数据	->	计算数据 -> 写回文件(打印出来)
2. 输入数据	->	计算数据 -> 写回文件(打印出来)
提高了cpu的利用率

并行:同一时刻执行多项任务，依赖的是多个cpu
并发:同一时间段执行多项任务
```

![image-20260417140911594](E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\image-20260417140911594.png)

### 1.进程的概念

- 进程是一个程序的执行过程，当程序执行后，则进程会产生，执行过程结束了，则进程结束

  ```
  进程是一个程序的执行过程，同一个程序每次运行的结果不一定一样
  ```

- 系统的每一个程序都会运行在某一个进程的上下文中

  <img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776390139166.png" alt="1776390139166" style="zoom:50%;" />

- 进程的特点

  1.进程是一个独立的可调度的活动，由操作系统进行统一的管理，相应的任务就会被cpu调度

  <img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776390298836.png" alt="1776390298836" style="zoom:50%;" />

  2.进程一旦产生，则需要分配相关的资源，同时进程是分配资源的最小单位

  <img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776390795556.png" alt="1776390795556" style="zoom:50%;" />

### 2.进程和程序的区别

-   程序是静态的概念，进程是动态的概念
-   进程是一个程序的一次执行活动
-   进程是一个独立的活动单位，是竞争资源的基本单位

### 3.进程的状态

```c
1.可执行程序是存放在硬盘，运行需要把程序放到内存，进程正在被创建
	操作系统会为进程分配资源
2.当进程完成创建后，会进入到就绪态
	已经具备了运行的条件，但是由于没有空闲的cpu(为别的进程服务)，暂时不能运行
3.当cpu空闲，操作系统就会选择一个没有就绪的程序，让它上cpu
	如果一个进程此时在cpu运行，就是处于运行态
4.进程运行过程中，可能会请求等待某个外部事件的发生(如等待某种系统资源的分配，等待其他的进程的响应)
	阻塞态(睡眠态)
5.一个进程结束了，进入到终止态(让进程下cpu,并且回收内存资源)
```

<img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776391646268.png" alt="1776391646268" style="zoom:50%;" />

拓展:一般会把操作系统分成两类

```c
1.分时系统
	以时间片轮转为主，每个进行执行一段时间(时间片)
	eg：
		大部分的桌面系统
		linux,windows,IOS,MACOS,安卓....
2.实时系统
	调度的策略主要以实时策略为主
	实时策略:
		每次调度都取优先级最高的那个进程执行，直到该进程执行完毕/主动放弃cpu/有更高的优先级进程(抢占/插队)
	eg:
		ucos/freeRTOS
```



### 4.linux进程地址空间

<img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776392263542.png" alt="1776392263542" style="zoom:50%;" />

- 一旦进程创建好后，系统则要为这个进程分配相应的资源，一般系统会每个进程分配4G的地址(32位系统)

- 4G地址空间结构

  <img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\1776393238815.png" alt="1776393238815" style="zoom:50%;" />

  ```
  为什么要分成用户空间和内核空间?
  	为了保护内核的安全，任何人都能修改内核的数据，行为很危险，会造成系统的奔溃
  	当用户进程需要内核获取资源的时候(例如获取磁盘中的数据)，会切换成内核态运行，这就会使当前的进程获取内核空间的资源
  ```

- 会为进程分配4G的虚拟地址空间

  ```
  虚拟地址空间中每一个地址都是虚拟地址
  
  虚拟地址:并不是真的内存空间，是用来寻址的编号
  物理地址:内存设备中真实存在的存储空间中编号
  ```

  <img src="E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\image-20260417110618016.png" alt="image-20260417110618016" style="zoom:50%;" />

- 虚拟地址通过映射的方式建立物理地址的关系，从而达到访问虚拟地址就可以访问到对应的物理地址

  ```
  在cpu中有个硬件MMU(内存管理单元)，负责虚拟地址与物理地址的映射管理以及虚拟地址访问
  
  使用虚拟地址原因：
  	直接访问物理地址，会导致地址没有隔离，很容易导致数据的被修改
  	每个进程的虚拟地址空间相互独立的，操作系统会映射到不同的物理地址区间中，再访问的时候就不会相互干扰
  	物理地址使用有限
  ```

  ![image-20260417111413174](E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\image-20260417111413174.png)

### 5.进程的创建

进程号类型为pid_t

调用fork函数，则会产生一个新的进程

调用fork的那个进程叫做父进程，产生的新的进程称为子进程

```c
#include <sys/types.h>
#include <unistd.h>

pid_t fork(void);
返回值：
	成功
		子进程返回值0
		父进程返回子进程的id  >0
	失败
		返回-1
```

参考示例

```
#include <sys/types.h>
#include <unistd.h>

int main()
{
    fork();
	fork();
	printf("hello\n");
}

结果打印4次hello
fork两次一共有4个进程，每个进程都打印一次hello
```

#### 5.1区分父子进程

注意调用一次fork会有两个返回
两次返回值的区别，子进程返回0 父进程返回子进程id

getpid：获取自己的进程号

getppid:获取调用这个函数的进程的父进程号

eg:子进程循环打印aaaa,每隔一秒打印一次 父进程循环打印bbbb,每隔一秒打印一次

```c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子：%d\n",getpid());
        printf("父：%d\n",getppid());
        // while(1)
        // {
        //     printf("aaaa\n");
        //     sleep(1);
        // }
    }
    else if(pid > 0)
    {
        printf("pid = %d\n",pid);//打一下pid的值 看一下是否跟子进程id一致
        //父进程
        // while(1)
        // {
        //     printf("bbbb\n");
        //     sleep(1);
        // }
    }    
}
```

#### 5.2父子进程的关系

```
一个进程里面包含哪些内容?
-系统数据
-数据(用户数据)
-指令(代码)

fork这个函数在创建子进程的时候，拷贝父进程的数据和指令
	整个进程地址空间中：父进程的变量，数据对象，标准IO缓冲区，文件描述符，进程的堆栈....
	copy了父进程的所有(目前可以这么认为)

子进程所独有的，只有它的进程号，进程的状态,...(系统数据)

使用fork函数代价比较大
```

#### 5.3父子进程地址空间

==父子进程地址空间是相互独立的==

创建一个子进程，并定义一个全局变量global = 0,在子进程中修改值为100，在父进程中打印global的值，思考为什么是这样的结果

```c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
int global = 0;
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        global = 100;
        while(1)
        {
            printf("子：%d %p\n",global,&global);
            sleep(1);
        }
    }
    else if(pid > 0)
    {
        //父进程
        while(1)
        {
            printf("父：%d %p\n",global,&global);
            sleep(1);
        }

    }    
}
```

思考

1.fork一旦成功，父子进程先执行哪一个?

```
操作系统调度
```

2.fork之后，子进程拷贝父进程的数据和指令，到底拷贝了什么数据?

```
-父进程全部的用户数据(局部变量 全局变量 静态变量...)
-标准IO缓冲区
-父进程打开的文件描述符和状态
-信号处理的方式(进程间通信)
-...
```

3.分析结果

```
int main()
{
	printf("abc");
	fork();
}
//abcabc

int main()
{
	printf("abc\n");
	fork();
}
//abc换行
```



### 6.进程的退出

进程退出两种情况

- 自杀(自己退出)

  - main函数的返回退出结束    会做清理工作，会把缓冲区的内容同步到终端

    用法：return n;   

  - 进程执行exit 结束进程   会做清理工作，会把缓冲区的内容同步到终端

    头文件：\#include <stdlib.h>    标准库函数

    用法：exit(n);  

  - 进程执行\_exit/_Exit 结束进程   不会做清理工作，会把缓冲区的内容直接丢弃

    头文件：\#include <unistd.h>     linux系统调用函数

    用法：_exit(n);  

- 它杀(操作系统)

  一个进行 可以 杀死另外一个进程(信号)

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main()
{
    printf("abc");
    #if 0
    _exit(0);//等价于 _Exit(0)  不会做清理工作，会把缓冲区的内容直接丢弃
    #else
    exit(0);//等价于 return 0;  会做清理工作，会把缓冲区的内容输出到终端
    #endif
}
```

### 7.等待子进程退出

wait/waitpid：用来回收子进程的资源

这两个函数用来等待某个子进程退出状态发生改变的，等待的状态发生变化有三种情况

-   子进程退出(正常退出)，main函数返回return/exit/_exit
-   子进程被信号终止
-   子进程被信号唤醒(blocking阻塞->ready)

```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t wait(int *wstatus);
@wstatus:指向的空间，保存子进程的退出信息(怎么死的，退出码)
返回值：
	成功：返回退出的那个进程的id
	失败：返回-1

eg:不想知道子进程怎么死的，只需要回收子进程的资源
	wait(NULL);
eg:想知道怎么结束子进程的
	int wstatus;
	wait(&wstatus);

	wstatus用来保存退出的那个子进程的退出信息的，退出信息保存到一个整数里面
	可以用以下宏解释子进程的退出信息
		WIFEXITED(wstatus)
			如果子进程正常终止(exit/_exit或main返回)，则返回真
		WEXITSTATUS(wstatus)
			返回子进程退出状态(退出码)，只有当WIFEXITED(wstatus)为真的时候，才能用这个宏
		WIFSIGNALED(wstatus)
			如果子进程因信号而终止，则返回真
		WTERMSIG(wstatus)
			返回导致子进程终止信号的编号，只有当WIFSIGNALED(wstatus)为真的时候，才能用这个宏
```

练习：创建子进程，子进程先打印自己的pid，接着循环1s打印一次数据aaa，父进程使用wait函数等待子进程结束，判断子进程是否是因信号而终止，如果是，则打印终止的那个信号的编号

把这个程序写完后,打开终端，运行，记住打印的pid,接着打开一个新的终端，输入 kill -9 子进程pid

```c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子：%d\n",getpid());
        while(1)
        {
            printf("aaa\n");
            sleep(1);
        }
    }
    else if(pid > 0)
    {
        //父进程
        //等待子进程结束
        int wstatus;
        wait(&wstatus);       
        
        if(WIFEXITED(wstatus))//判断是否正常结束子进程
        {
            //打印退出码
            printf("exit code:%d\n",WEXITSTATUS(wstatus));
        }
        if(WIFSIGNALED(wstatus))//判断子进程是否因信号而终止
        {
            printf("signal number:%d\n",WTERMSIG(wstatus));
        }
    }    
}
```

waitpad

```c
#include <sys/types.h>
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *wstatus, int options);
@pid:指定要等待的进程/进程组
	pid < -1
  		表示等待进程组 ID等于该 pid 绝对值的任意子进程。
  		-1234:表示等待1234那个进程组的所有子进程
	pid = -1
  		表示等待任意子进程

	pid = 0
  		表示等待进程组 ID 与调用进程相同的任意子进程。

	pid > 0
  		表示等待进程 ID 等于该 pid 值 的子进程。	
@wstatus:指向的空间，用来保存子进程退出信息
@options：等待选项
	0			默认阻塞等待
	WNOHANG		非阻塞等待，假如没有子进程退出，则立即返回
返回值：
	成功：返回退出的那个进程的id
	失败：返回-1

eg:
	wait(&wstatus) <==> waitpid(-1,&wstatus,0)
```

进程组

```c
就是一组进程，每个进程会属于某个进程组
并且每个进程组，都会有一个组进程
创建这个进程组的进程就是组长，经常组有一个组id(组长的pid)

A创建B和C，B创建D
进程创建的关系
	A(pid=1000,pgid=1000)
		B(pid=1001,pgid=1000) 默认继承关系
			D(pid=1003,pgid=1000) 默认继承关系
		C(pid=1002,pgid=1000) 默认继承关系
进程创建的关系
	A(pid=1000,pgid=1000)
		B(pid=1001,pgid=1001) <--新进程组 setpgid
			D(pid=1003,pgid=1001) <--继承B进程组
		C(pid=1002,pgid=1000) <--继承A进程组
```

练习：子进程立即结束，父进程一直死循环，不调用wait,建议把子进程和父进程的id打印

写完代码运行后，打开新的终端，使用ps -aux | grep  Z 查看进程，试一下，能不能把子进程杀死

```c
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main()
{
    pid_t pid = fork();
    if(pid == 0)
    {
        //子进程
        printf("子:%d\n",getpid());
        exit(0);
    }
    else if(pid > 0)
    {
        //父进程
        printf("父:%d\n",getpid());
        while(1);

    }    
}
```

僵尸进程：

```
子进程终止，父进程没有回收子进程的资源，子进程就会变成僵尸进程
```

孤儿进程

```
父进程运行结束，但是子进程还在运行，这个子进程就会变成孤儿进程
```

### 8.exec函数簇

eg:另外一个程序

```c
#include <stdio.h>
#include <stdlib.h>

//gcc 08-test.c -o 08-test
//./08-test 3 4
int main(int argc,char **argv)
{
    printf("%s + %s = %d\n",argv[1],argv[2],atoi(argv[1])+atoi(argv[2]));
}
```

进程替换

```c
exec函数簇 是让一个程序去执行另外一个程序的文件

exec让一个进程去执行另外一个程序文件
	需要指定这个程序文件的名字
		一个文件系统中程序文件的名字(带路径)
	还得指定程序运行所需要的参数
		对于linux程序的参数，都是当成字符串来处理的
	
	指定参数的方式有两种
		l:list
			把程序运行的参数，一个一个列举出来
			"08-test","3","4",NULL
		v:vector 向量(数组)
			把程序运行的参数，弄成一个char*数组
			char *arg[] = {"08-test","3","4",NULL};
```

```c
#include <unistd.h>

extern char **environ;

int execl(const char *path, const char *arg, ...
/* (char  *) NULL */);
@path:你要执行的程序文件的名字(带路径)
@arg,...
	程序运行的参数  list->以列表的方式指定
返回值：
	成功 永远都不返回!!!因为当前的进程的整个数据和指令都被别人替换了，没得返回
	失败 -1
	
int execlp(const char *file, const char *arg, ...
/* (char  *) NULL */);
@path:你要执行的程序文件的名字(不带路径)(会在PATH路径下找)
@arg,...
	程序运行的参数  list->以列表的方式指定
返回值：
	成功 永远都不返回!!!因为当前的进程的整个数据和指令都被别人替换了，没得返回
	失败 -1
int execv(const char *path, char *const argv[]);
@path:你要执行的程序文件的名字(带路径)
@argv
	程序运行的参数  v->以数组的方式指定
返回值：
	成功 永远都不返回!!!因为当前的进程的整个数据和指令都被别人替换了，没得返回
	失败 -1
int execvp(const char *file, char *const argv[]);
@path:你要执行的程序文件的名字(不带路径)(会在PATH路径下找)
@arg,...
	程序运行的参数  v->以数组的方式指定
返回值：
	成功 永远都不返回!!!因为当前的进程的整个数据和指令都被别人替换了，没得返回
	失败 -1
```

eg:

在子进程中查看当前目录下的文件详细信息

在终端执行命令

```
ls -l
```

=>ls程序文件实在PATH指定的目录下"/bin/ls"

```
execl("/bin/ls","ls","-l",NULL);
execl("/mnt/hgfs/CS2612/三阶段/01-并发/01-进程/02-code
/08-test","./08-test","3","4",NULL);
```

或

```
execlp("ls","ls","-l",NULL);//ls的/bin路径需要在PATH路径中
execlp("08-test","./08-test","3","4",NULL);

添加环境变量
export PATH=$PATH:你的路径
```

![image-20260417173932299](E:\A_yueqianStudy\三阶段\01-并发\01-进程\01-note\进程.assets\image-20260417173932299.png)

使用execv和execvp实现程序

```c
#include <stdio.h>
#include <unistd.h>
int main()
{
    #if 0
    int ret = execl("/mnt/hgfs/CS2612/三阶段/01-并发/01-进程/02-code/08-test","./08-test","3","4",NULL);
    if(ret == -1)
    {
        perror("execl failed");
        return -1;
    }
    printf("ret == %d\n",ret);
    #endif 

    #if 0
    execlp("08-test","./08-test","3","4",NULL); //路径一定要加环境变量
    #endif  

    #if 0
    char *arg[] = {"./08-test","3","4",NULL};
    int ret = execv("/mnt/hgfs/CS2612/三阶段/01-并发/01-进程/02-code/08-test",arg);
    if(ret == -1)
    {
        perror("execl failed");
        return -1;
    }
    printf("ret == %d\n",ret);
    #endif 

    #if 1
    char *arg[] = {"./08-test","3","4",NULL};
    int ret = execvp("08-test",arg);
    if(ret == -1)
    {
        perror("execl failed");
        return -1;
    }
    printf("ret == %d\n",ret);
    #endif 

}
```



### 9.system

```c
#include <stdlib.h>

int system(const char *command);
功能：执行command这个字符串命令
		shell命令(只要是终端上能输入的命令都可以)
		执行完这个命令后，再进行返回，执行system后面的代码
@command:命令

eg:
	system("ls -l");
	system("./a.out 3 7");
```

作业：

1.程序执行完成后，输出的结果是多少,分析原因

```
int main()
{
	int i;
	for(i = 0;i<2;i++)
	{
		fork();
		printf("_\n");
	}
}

6个
```

2.程序执行完成后，输出的结果是多少,分析原因

```c
int main()
{
	int i;
	for(i = 0;i<2;i++)
	{
		fork();
		printf("_");
	}
}

8个
```

3.写一个madplay的程序，播放一个指定目录下所有的mp3文件

```c
注意：一首歌结束后才能播放下一首个，使用wait函数
父进程负责创建进程，并等待子进程的结束
子进程负责播放音乐，播放歌曲结束后由父进程回收


sudo apt install madplay 要联网

system("madplay /lixiang/music/*.mp3 &");//播放/lixiang/music所有的mp3文件 并且后台播放
system("madplay /lixiang/music/*.mp3 -r &");//列表循环
system("killall -STOP madplay");//暂停播放
system("killall -CONT madplay");//继续播放
system("killall -9 madplay");//停止播放播放
```

```C
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char *mp3[] = {"1.mp3","2.mp3","3.mp3"};
    int i=0;
    while(1)
    {
        //创建进程
        pid_t pid = fork();
        if(pid == 0)
        {
            //子  负责播放音乐
            char cmd[100] = {0};//用来保存要执行的命令
            sprintf(cmd,"madplay %s",mp3[i]);//拼接命令
            printf("正在播放%s\n",mp3[i]);
            //system(cmd);//执行命令
            execl("/usr/bin/madplay","madplay",mp3[i],NULL);
            exit(0);//结束
        }
        else if(pid > 0)
        {
            //父 等待子进程结束
            wait(NULL);
            printf("%s已经播放完毕\n",mp3[i]);
            i++;
            if(i == 3)
            {
                printf("当前列表已经播放完毕\n");
                break;
            }
        }
    }
}
```

总结

```
1.进程的概念
	程序：文件
	进程：运行的时候才会产生，会产生一块区域
2.进程的状态切换
	ready
	running
	blocking(sleping/waitting)
3.调度策略
	分时系统:linux,windows  时间片
	实时系统：优先级
4.fork创建子进程
	子 返回0
	父 返回子进程的pid
	父子进程地址空间相互独立的
5.子进程的回收资源
	父进程wait  waitpid
	僵尸进程:子进程退出后，父进程没有回收子进程的资源，子进程就会变成僵尸进程
	孤儿进程:父进程先退出，留下了子进程
6.进程退出
	return/exit 缓冲区如果有数据，退出后，会把缓冲区的结果进行同步
	_exit/_Exit 缓冲区如果有数据，退出后，会把缓冲区的结果丢弃
7.exec函数簇
	进程替换
	execl
	execlp
	execv
	execvp
	l:list   一一列举出来
		./a.out abc  aaa
		"./a.out","abc","aaa",NULL
	v：向量 数组
		char *arg[] = {"./a.out","abc","aaa",NULL};
	p：环境变量PATH
		export PATH=$PATH:/home/china
```

# 二、进程间通信

## 1、管道

### 1.无名管道pipe

它在文件系统中没有名字，它的内容在内核(操作系统中使用一块内存)，访问无名管道通过文件系统API(read/write)

在创建无名管道的时候，就需要返回文件描述符

无名管道在创建的时候，返回两个文件描述符(一个是用来读的，一个是用来写的)

```
#include <unistd.h>

int pipe(int pipefd[2]);
功能：创建无名管道
参数：
	pipefd:用来保存无名管道读和写的文件描述符
		pipefd[0]:读		read(pipefd[0],buf,10);
		pipefd[1]:写		write(pipefd[1],buf,10);
返回值：
	成功 0
	失败 -1
eg:
	int pipefd[2];
	pipe(pipefd);
```

特点:

```c
1.内容读走了，就没有了
2.按顺序读，不支持lseek(先进先出)
3.如果写端没有关闭，管道没有数据，读管道进程去读管道会阻塞
    如果写端没有关闭，管道有数据，读管道进程去读管道就会将数据读取出来，下一次读没有数据就会阻塞
4.如果全部写端被关闭，读进程去读管道的数据，读取了全部的内容之后，如果再读，最后返回0
5.管道所有的读端都被关闭，写管道进程写管道收到一个信号(SIGPIPE),退出
```

注意：

```c
1.pipe原则上可以用于任意的进程间的通信，只要通信的进程能够获取同一个pipe的文件描述符。但是一般情况下我们将pipe用于有亲缘关系的进程间通信
2.pipe本身是一个全双工通信，但是两个进程利用无名管道去实现全双工的通信可能会出问题，自己很有可能读到自己写到无名管道的内容，一般无名管道人为的人为是半双工
```



### 2.有名管道fifo

fifo是pipe的基础上，再给fifo文件在文件系统中创建了一个inode结点(它在文件系统有名字)，但是fifo的内容还是存在内核中

fifo的文件名随文件系统持续存在，内容是随进程持续存在

```c
#include <sys/types.h>
#include <sys/stat.h>

int mkfifo(const char *pathname, mode_t mode);
@pathname:要创建的有名管道的路径
	注意：不要在共享文件夹中进行创建，会没有权限
		管道文件只有在linux下才有，不要把你创建的管道文件放在共享文件夹下面
		"/home/china/1.fifo"
@mode:要创建的有名管道的权限，有两种
	1.宏
	2.八进制
		0777
返回值：
	成功 0
	失败 -1
eg:
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
    //创建管道
    int ret = mkfifo("/home/china/1.fifo", 0777);
    if(ret == -1)
    {
        if(errno == EEXIST)//已经存在
        {
            printf("文件已存在，🙅不要创建\n");
        }
        else
        {
            perror("mkfifo failed");
            return -1;
        }
    }

    //已经存在就直接做后面的事情
    int fd = open("/home/china/1.fifo",2);

}	
```

还可以再命令行创建

```
mkfifo 你要创建的管道文件的名字(带路径)
eg:
	mkfifo /home/china/cs2612.fifo
```

## 2、信号

### 1.信号的概念

- 信号是进程与进程通信的方式之一，这种方式不能传输数据的

  只是在内核中传递一个信号(整数)

- 不同的信号值，所代码的含义是不同的

- 信号本质就是软件中断，同时信号异步(不知道信号什么时候来)

  信号会中断正在执行的程序，转而去执行中断函数，处理完中断后，再继续执行原来的程序代码

  ![image-20260420155730266](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\02-进程间通信\2.信号\01-note\信号.assets\image-20260420155730266.png)

查看信号的值

```
kill -l
trap -l
```

当一个进程收到一个信号，可能发送5种默认的行为  man 7 signal

```
Term   Default action is to terminate the process.
		默认行为终止进程

Ign    Default action is to ignore the signal.
		忽略信号	

Core   Default action is to terminate the process and  dump  core  (see
core(5)).
		输出信号，然后终止进程

Stop   Default action is to stop the process.
		停止进程

Cont   Default  action  is  to  continue the process if it is currently
stopped.
		如果进程当前停止，则继续该进程

注意：如果用户没有显示的处理信号，系统的默认方式大多数都是终止进程
```

<img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\02-进程间通信\2.信号\01-note\信号.assets\image-20260420161012585.png" alt="image-20260420161012585" style="zoom: 67%;" />

注意：

```
SIGKILL SIGSTOP 不能被捕捉 不能被阻塞 不能被忽略
```

### 2.信号三种处理方式

进程在收到一个信号后，通常有3种处理方式

1.捕捉信号

```
把一个信号 与用户自定义的信号处理函数 关联起来
那么在收到信号的时候，就会自动调用自定义的信号处理函数
```

```
#include <signal.h>

typedef void (*sighandler_t)(int);//函数指针，要传函数的名字，规定函数参数的类型为int,返回值为void

sighandler_t signal(int signum, sighandler_t handler);
功能：设置接受到某种信号后的处理方式
@signum:要捕捉到的那个信号的值
@handler：指定信号的处理方式
		1)自定义的处理函数名
		2)默认行为
			SIG_DEF
		3)忽略
			SIG_IGN
返回值：
	返回该信号上一次的处理方式(第一次返回NULL)
```

eg:

```
void handler(int signum)//参数就是信号的值
{
	
}

int main()
{
	signal(SIGINT,handler);
	while(1);
}
```

2.默认行为

```
收到一个信号时，采用操作系统的默认行为
大部分信号的默认行为，会把进程终止
只有一个信号SIGCHLD，收到时会忽略行为
```

3.忽略该信号

```
相当于没有收到
```

## 3、system V

SYSTEM V是一些unix系统特性的标准，也就是早期就常出现的一些通信标准

提供了成套的IPC通信接口，包含

>   消息队列 message queue
>
>   共享内存share memory
>
>   信号量semaphore

system V设计理念：在所有进程都能访问的内核空间中，开辟并维护一个相关的结构体空间，用来服务应用程序之间的数据的传递

只不过对于它的访问不需要借助文件IO->系统调用，还是需要时间!!!

主张是让上层应用程序 直接 访问 具备内核空间权限的东西，要怎么实现>

=>应用程序是没有权限访问内核空间的

=>许可证key

### 1.IPC_key

进程间通信许可证是由内核向应用程序颁布的，用于证明应用程序之间拥有借助内核空间进行数据传输资格的一种许可证

```c
#include <sys/types.h>
#include <sys/ipc.h>

key_t ftok(const char *pathname, int proj_id);
@pathname:一个文件系统的路径名(必须要是存在的而且有读的权限，在linux文件夹下)
	"/home/china"
@proj_id:整数，这个参数存在的意义是让一个路径也能生成多个key
	ftok只取proj_id的低八位
	eg:
		1
返回值：
	成功，返回一个唯一的system V ipc的key
	失败，-1
eg:
	key_t key = ftok("/home/china",1);
	if(key == -1)
	{
		
	}
```

2.共享内存

![image-20260421093334192](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\02-进程间通信\3.system V\02-note\system V.assets\image-20260421093334192.png)

**a.ftok生成许可证**

**b.shmget创建或打开system V共享内存**

```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
@key：由ftok获取
@size：以字节为单位指定共享内存的大小(如果是打开，可以给0)，一般给1024的整数倍
@shmflg：标志位，指定创建或打开共享内存的方式
	如果是创建共享内存:IPC_CREAT |  权限位(0666)
		eg:
			IPC_CREAT | 0777
		IPC_EXCL:测试标志，通常和IPC_CREAT一起用
			IPC_CREAT | IPC_EXCL 如果key对应的共享内存不存在，则创建
								  如果key对应的共享内存存在，则失败
返回值：
	成功，返回共享内存的id,这个id就唯一表示这个共享内存
	失败,-1
eg:
	int shm_id = shmget(key,4096,IPC_CREAT | IPC_EXCL | 0777);
	if(shm_id == -1)
	{
		//如果是因为共享内存存在而失败，则直接打开
		if(errno == EEXIST)
		{
			shm_id = shmget(key,0,0);
		}
		else 
		{
			perror("shmget failed");
			return -1;
		}
	}
```

**c.shmat映射到通信进程的各自的进程地址空间中去**

```c
#include <sys/types.h>
#include <sys/shm.h>

void *shmat(int shmid, const void *shmaddr, int shmflg);
@shmid:要映射的共享内存的id(shmget)
@shmaddr:指向要银蛇到进程的哪个地址上去
	一般填NULL，有系统分配
@shmflg：
	SHM_RDONLY 只读
	0 可读可写
返回值：
	成功 返回映射后的地址
	失败 (void *) -1 
eg:
	int *p = shmat(shm_id,NULL,0);
	if((void*)p == (void*)-1)
	{
		
	}
```

**d.通信**

```c
int *p = shmat(shm_id,NULL,0);
*p = 3;//(一个程序写入)
printf("%d\n",*p);//(另外一个程序读取)

char *p = shmat(shm_id,NULL,0);
strcpy(p,"nihao");//(一个程序写入)
printf("%s\n",p);//(另外一个程序读取)
```

**e.shmdt解映射**

```
#include <sys/types.h>
#include <sys/shm.h>

int shmdt(const void *shmaddr);
@shmaddr:需要接触映射的地址

eg:
	shmdt(p);	
```

**f.shmctl删除共享内存**

```c
#include <sys/ipc.h>
#include <sys/shm.h>

int shmctl(int shmid, int cmd, struct shmid_ds *buf);
@shmid：要删除的共享内存的id(shmget)
@cmd:控制命令，不同的命令对应的第三个参数的值会不一行
	IPC_RMID
@buf 保存 第二个参数相关的数据 struct shmid_ds *
	如果cmd == IPC_RMID buf填NULL
返回值
	成功 0
	失败 -1
eg:
	//删除共享内存
	shmctl(shm_id,IPC_RMID,NULL)
```

### 3.信号量

在并发执行多个进程的时候，由于系统资源存在公共资源，并且由些公共资源对访问的数量有限制，因为当多个进程同时访问一个 有限制的共享资源的时候，避免不了竞争

信号量本质：数字，在访问受限制的共享资源之前，必须先访问并获得信号量，如果无法获取信号量，只能进入等待或放弃对共享资源的访问

信号量->厕所门的锁

一个进程或一个线程可以在某个信号量上执行三个操作

- 创建信号量，要求创建者立马指定信号量的初值

- 等待一个信号量(lock) p上锁

  ```c
  该操作需要测试这个信号量是否被别人获取了
  	如果信号量已经被别人获取，则进入等待
  	如果信号量没有被别人获取，则获取信号量 =>
  		你就可以访问这个资源
  		访问资源的代码
  
  p 
  操作资源(*p)++   ->临界区
  V
  ```

- 释放一个信号量(unlock) v解锁

  ```
  当你获取信号量之后，并访问完共享资源后，需要立即释放信号量
  =>因为别人可能在等待信号量
  ```

p操作作为可访问资源减操作，v操作作为可访问资源增加操作

#### 3.1 信号量相关的API函数

**a.ftok：获取system V ipc对象的key**

**b.semget:用来创建或打开一个system V信号量集**

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semget(key_t key, int nsems, int semflg);
@key:ftok获取
@nsems：你要创建的信号量集合中信号量的个数
@semflg：标志位
	创建：IPC_CREAT|权限位(0777)
		IPC_EXCL
		有两种情况
		1.成功
			信号量集合不存在，创建一个信号量集合
		2.失败
			失败的原因errno == EEXIST
			已经创建过了
			之后打开信号量集合
	打开 0
返回值：
	成功 返回一个信号量集合的id
	失败 -1
eg:
	int sem_id = semget(key,5,IPC_CREAT | IPC_EXCL | 0777);
	if(sem_id == -1)
	{
		//如果是因为信号量集合存在而失败，则直接打开
		if(errno == EEXIST)
		{
			sem_id = semget(key,5,0);//如果是打开会自动忽略第二个参数
		}
		else 
		{
			perror("semget failed");
			return -1;
		}
	}
```

**c.semctl：控制操作(设置或获取信号量集中某个或某些信号量的值)**

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semctl(int semid, int semnum, int cmd, ...);
@semid:要操作的信号量集合的id(semget)
@semnum:要操作的信号量集中的哪个信号量的下标[0,nsems-1]
@cmd:命令
	IPC_STAT 	获取属性
	IPC_SET		设置属性
	IPC_RMID	删除信号量集
	GETALL		获取信号量集中所有信号量的值
	SETALL		设置信号量集中所有信号量的值
	GETVAL 		获取下标semnum的信号量的值
	SETVAL 		设置下标semnum的信号量的值
	....
@...:根据不同的命令，第四个参数也会不一样
    union semun {
        int              val;    /* Value for SETVAL */
        struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
        unsigned short  *array;  /* Array for GETALL, SETALL */
        struct seminfo  *__buf;  /* Buffer for IPC_INFO
        (Linux-specific) */
    };
	cmd == IPC_RMID 不需要第四个参数
    	semctl(sem_id,0,IPC_RMID);
    cmd == SETVAL
    	int val = 1;
    	semctl(sem_id,0,SETVAL,val);
    	//设置sem_id所表示的信号量集合中下标为0的信号量的值为val
    cmd == GETVAL
    	int val = semctl(sem_id,0,GETVAL);
    	//val就表示semid所表示的信号量集合中下标为0的哪个信号量的值
    cmd == SETALL
    	unsigned short array[5] = {1,0, 0 ,0 ,0};
    	int val = semctl(sem_id,0,SETALL,array);
    cmd == GETALL
    	unsigned short array[5];//准备空间，用来保存信号量集的值
    	int val = semctl(sem_id,0,GETALL,array);
返回值：
    根据命令的不同，返回的结果的含义也不一样
    0  成功
    -1 失败
```

练习：创建一个system v信号量集合(5个信号量)，给所有的信号量设置初值，并且获取第三个信号量的值，并打印

**d.semop:PV操作**

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int semop(int semid, struct sembuf *sops, size_t nsops);
@semid:semget的返回值
@sops：指定信号量操作信息的结构体数组
	struct sembuf
	{
        unsigned short sem_num;  /* semaphore number */
        	//要进行操作的信号量在信号量集合中的下标
        short          sem_op;   /* semaphore operation */
        	/*
        		>0  v操作	unlock
        		==0 看是否阻塞 自己试一下
        		<0	p操作 lock
        	*/
        short          sem_flg;  /* operation flags */
        	/*
        		0	表示默认，如果p操作失败,就会阻塞
        		IPC_NOWAIT	非阻塞等待，不等待
        		SEM_UNDO	撤销，为了防止进程带锁退出
        					用于父子进程(没有血缘关系别用)
        	*/
    };
@nsops：指定第二个参数sops数组中元素的个数
返回值：成功0 失败-1

int semtimedop(int semid, struct sembuf *sops, size_t nsops,
const struct timespec *timeout);
限时等待：如果指定的时间内还没有p操作成功，直接返回
@timeout：指定限时的时间
    	struct timespec
        {
            time_t tv_sec;//秒
            long tv_nsec;//纳秒
        }
eg:显示等待5.2s
    struct sembuf buf;
	buf.sem_num = index;//要操作的信号量的下标
	buf.sem_op = -1;//资源数量-1
	buf.sem_flg = 0;//阻塞等待，直到拿到信号量为止
	
	struct timespec tv;
	tv.tv_sec = 5;
	tv.tv_nsec = 200000000;//1s = 1000ms = 1000 000us = 1000 000 000ns
    semtimedop(semid,&buf,1,&tv);
```

eg:

```c
/*
	p
	sem_id:semget获取的	
	index:要操作的信号量集中的哪个信号量的下标
*/
void sem_p(int sem_id,int index)
{
	struct sembuf buf;
	buf.sem_num = index;//要操作的信号量的下标
	buf.sem_op = -1;//资源数-1
	buf.sem_flg = 0;//阻塞等待，直到拿到信号量
	semop(sem_id, &buf, 1);
}

/*
	v
	sem_id:semget获取的	
	index:要操作的信号量集中的哪个信号量的下标
*/
void sem_v(int sem_id,int index)
{
	struct sembuf buf;
	buf.sem_num = index;//要操作的信号量的下标
	buf.sem_op = +1;//资源数+1
	buf.sem_flg = 0;//阻塞等待，直到拿到信号量
	semop(sem_id, &buf, 1);
}

使用
    //p 上锁
    sem_p(sem_id,0);
    //临界区->操作共享资源(*p)++
    //v 解锁
    sem_v(sem_id,0);
```

查看IPC

```
ipcs		#查看所有IPC对象
ipcs -a 	#查看所有IPC对象
ipcs -m		#查看共享内存对象
ipcs -s		#查看信号量对象
```

删除IPC对象

```
ipcrm -M key	#根据指定键值key,删除指定的共享内存
ipcrm -m id		#根据id,删除指定的共享内存

ipcrm -S key	#根据指定键值key,删除指定的信号量
ipcrm -s id		#根据id,删除指定的信号量
```

思考题：

1.在遇到共享内存在不同的进程或线程中访问的时候，考虑避免竞争，用什么方式访问

```c
1.使用顺序执行(不会并发)
2.信号量
	(1)明确共享资源是谁
	(2)确定临界区
	(3)一个要保护的对象，就需要用信号量
```

2.现在有5个共享资源ABCDE，需要被保护，决定用一个信号量来保护5个共享资源，这样设计有什么问题

```
p(m)
ABCDE资源
V(m)
```

3.现在有5个共享资源ABCDE，需要被保护，决定用五个信号量来保护5个共享资源，这样设计有什么问题

```
m1->A
m2->B
...

p(m1)
A资源
v(m1)
p(m2)
B资源
v(m2)
....
```

```c
假设需要同时使用A和B
	进程1		进程2
    p(m1)	p(m2)
    A资源	   B资源
    v(m1)	v(m2)
    p(m2)	p(m1)
    B资源	   A资源
    v(m2)	v(m1)
    
	进程1		进程2
    p(m1)	p(m2)
    p(m2)	p(m1)
    A资源	   B资源
    B资源	   A资源
    v(m2)	v(m1)
    v(m1)	v(m2)
=>死锁：所谓死锁是指多个进程因竞争资源而相互等待，若无外力作用，这些进程都无法向前推进。
	产生死锁的原因：
		1.因为系统资源的不足
		2.程序运行推进的顺序不合理
		3.资源分配不当
	产生死锁的四个必要条件：
        1.互斥条件 ：  一个资源只能被一个进程所占用，此时若其它进程请求该资源，则请求进程必须等待（比如一个男人只可以和一个女人结婚，不可能同时跟七八个女的一起结）
    	2.不剥得条件 ：  进程使用的资源在未完成使用之前，不能被其它进程强行得走。（在一个人未离婚之前不能再跟其他人结婚）
    	3.请求和保持条件 ：  一个进程因为请求资源而阻塞时，对已获资源保持不放（妥妥海王，还没追到另一个女生之前，还是不把已经撩到的七八个女生放手，硬要接着养鱼）
    	4.循环等待条件 ：   若干进程形成了一种头尾相接环状等待资源的关系（三角恋，我等你，你等他，他又等我）
	避免死锁：上面四个必要条件，只要有一条不满足，就不会导致死锁(银行家算法:课后去了解)
```

4.若系统有一个资源，资源数为7，有多个进程均需要使用2个资源，规定每个进程一次仅允许使用1个资源，则最多允许多个进程参与竞争，而不会发生死锁(哲学家就餐)

## 4、posix信号量

### 1.posix信号量

有名信号量

无名信号量

a.创建或打开一个posix信号量

>   (1)sem_open创建或初始化一个posix有名信号量
>
>   ```c
>   #include <fcntl.h>           /* For O_* constants */
>   #include <sys/stat.h>        /* For mode constants */
>   #include <semaphore.h>
>   
>   sem_t *sem_open(const char *name, int oflag);
>   sem_t *sem_open(const char *name, int oflag,
>   mode_t mode, unsigned int value);
>   @name:指定要创建或打开的posix信号量在文件系统中的路径名，一定要以"/"开头
>   	eg:"/home" "/data" "/dev" 存在与否都可以
>   @oflag
>   	(1)创建 O_CREAT
>   		判断文件是否存在
>   		O_CREAT | O_EXCL
>   	(2)打开 0
>   第三个参数和第四个参数  当第二个参数是 创建一个有名信号量的时候，才需要
>   @mode：权限
>   	1.宏
>   	2.八进制 0777
>   @value：指定信号量的值
>   返回值：
>   	成功返回sem_t类型的指针，指向一个已经打开的posix信号量
>   	失败 返回SEM_FAILED
>   Link with -pthread.
>   编译需要加 -pthread
>   gcc xxx.c -l pthread
>   
>   eg:
>   	sem_t*sem = sem_open("/home",O_CREAT | O_EXCL,0777,1);
>   	if(sem == SEM_FAILED)
>   	{
>   		if(errno == EEXIST)//存在
>   		{
>   			//打开的方式
>   			sem = sem_open("/home",0);
>   		}
>   		else
>   		{
>   			perror("sem_open failed");
>   			return -1;
>   		}
>   	}
>   ```
>
>   (2)sem_init初始化一个posix无名信号量
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_init(sem_t *sem, int pshared, unsigned int value);
>   @sem：要初始化的posix无名信号量的首地址
>   	1.sem_t sem;//传参数&sem
>   	2.sem_t *sem=malloc(sizeof(sem_t));//传参数sem
>   @pshared:指定无名信号量的共享方式
>   	0：进程内部的线程共享，sem指向的是进程内部的共享区域
>   	1(非0)：不同进程间的共享，sem指向的内核共享的区域 <---------
>   @value:设置信号量的初值
>   返回值：
>   	成功 0
>   	失败 -1
>   Link with -pthread.
>   编译需要加 -pthread
>   
>   eg:
>   	sem_t sem;
>   	int ret = sem_init(&sem,1,1);
>   	if(ret == -1)
>   	{
>   
>   	}
>   ```
>
>   

b.posix信号量P/V操作

>   1.p操作sem_wait
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_wait(sem_t *sem);//阻塞函数  上锁p
>   返回值：
>   	成功0
>   	失败-1
>   
>   int sem_trywait(sem_t *sem);//非阻塞
>   返回值：
>   	成功0
>   	失败-1
>   
>   int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);//限时等待
>   @abs_timeout:获取当前的时间(1970.1.1经过的秒数)+愿意等待的时间
>   
>   Link with -pthread.
>   ==================================
>   #include <time.h>
>   
>   
>   int clock_gettime(clockid_t clk_id, struct timespec *tp);
>   功能：获取时间(1970.1.1)
>   @clk_id：CLOCK_REALTIME
>   @tp:保存获取的时间
>   Link with -lrt 
>   
>   eg:
>   	愿意等待5.2s
>   	//获取当前的时间
>   	struct timespec tp;
>   	clock_gettime(CLOCK_REALTIME,&tp);
>   	tp.tv_sec+=5;//秒
>   	tp.tv_nsec+=200000000;//纳秒
>   	if(tp.tv_nsec >= 1000000000)//进位
>   	{
>   		tp.tv_sec+=1;
>   		tp.tv_nsec-=1000000000
>   	}
>   	sem_timedwait(sem, &tp);//限时等待5.2s
>   ```
>
>   2.v操作 sem_post
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_post(sem_t *sem);
>   
>   Link with -pthread.
>   ```
>
>   

c.posix信号量其他操作

>   1.用来获取信号量的值
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_getvalue(sem_t *sem, int *sval);
>   
>   Link with -pthread.
>   eg:
>   	int val;
>   	sem_getvalue(sem, &val);//val就是你信号量当前的值
>   ```
>
>   2.posix有名信号量的关闭和删除
>
>   ```c
>   #include <semaphore.h>
>   int sem_close(sem_t *sem);
>   Link with -pthread.
>   ```
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_unlink(const char *name);
>   //name要填有名信号量的路径
>   Link with -pthread.
>   ```
>
>   3.无名信号量的删除
>
>   ```c
>   #include <semaphore.h>
>   
>   int sem_destroy(sem_t *sem);
>   Link with -pthread.
>   ```
>
>   

有名信号量

```
1.sem_open
2.pv操作(sem_wait  sem_post)
3.sem_close(关闭信号量)
4.sem_unlink(删除信号量)
```

无名信号量

```
1.sem_init
2.pv操作(sem_wait  sem_post)
3.sem_destroy(删除信号量)
```

## 5、线程

在linux，所有的调度的实体叫做任务。它们的区别：有些任务各自拥有一套资源(进程)，而有些任务共享一套资源(线程)

分析：

```
进程的地址空间是相互独立的
	进行通信，必须用到第三方，比较麻烦，通信的代码比较大
	ipc(管道，共享内存，信号，信号量)
	创建一个进程的系统开销比较大，子进程拷贝父进程所有的数据
```

线程是比进程更小的活动单位，它是进程的执行分支

进程内部可以有多个线程，它们并发执行，但是进程内部所有的线程共享整个进程的地址空间

main函数进程就是主线程，也就是说我们会有一个线程负责执行main函数，其他的线程就可以去执行其他的函数

==当主线程也就是main函数执行完毕后，不管其他的线程有没有结束，进程退出==

<img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\02-进程间通信\4.线程\01-note\线程.assets\image-20260422141729904.png" alt="image-20260422141729904" style="zoom:50%;" />

特点：

```
1.创建一个线程 比 创建一个进程开销要小很多
2.实现线程间的通信比较简单
3.线程是动态的概念
	线程状态图
	就绪 运行态 阻塞态(等待 睡眠)
4.资源分配的最小单位是进程，调度的最小单位是线程
```

### 1.linux下线程的API接口

#### 1.1创建线程

```c
#include <pthread.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
void *(*start_routine) (void *), void *arg);
功能：创建一个线程
@thread：指针，指向的变量用来保存新创建线程的信息，就是用来保存线程id的
@attr：线程属性，一般填NULL，默认属性
@start_routine：函数指针，指针对于的线程函数，开创线程的目的就是为了指向线程函数
@arg:线程函数的实际参数，函数不需要传参，填NULL
返回值：
	成功0
	失败-1
Compile and link with -pthread.
eg:
#include <unistd.h>
void *abc(void*arg)
{
	while(1)
	{
		printf("hello\n");
		sleep(1);
	}
}
int main()
{
	//创建一个线程
	pthread_t thread;
	pthread_create(&thread,NULL,abc,NULL);
	while(1);
}
```

#### 1.2线程的退出

1.线程调用return

2.在程序执行的任意时刻，调用pthread_exit

```c
#include <pthread.h>

void pthread_exit(void *retval);

Compile and link with -pthread.
eg:
	pthread_exit((void*)-1);
eg:
	int *a = malloc(sizeof(int));
	*a = -1;
	pthread_exit((void*)a);
```

3.被别的线程取消，别的线程调用pthread_cancel

```c
#include <pthread.h>
//取消线程
int pthread_cancel(pthread_t thread);

Compile and link with -pthread.
===========================================
//设置线程  能否被取消
#include <pthread.h>

int pthread_setcancelstate(int state, int *oldstate);
@state:指定要设置的属性
	PTHREAD_CANCEL_ENABLE	能被取消(默认属性)
	PTHREAD_CANCEL_DISABLE	不能被取消
@oldstate：指向的空间保存上一次 取消属性的状态，不想保存(NULL)
int pthread_setcanceltype(int type, int *oldtype);
@type:取消类型
	PTHREAD_CANCEL_DEFERRED  立即取消(默认)
	PTHREAD_CANCEL_ASYNCHRONOUS   延时取消
@oldstate：指向的空间保存上一次 取消属性的类型，不想保存(NULL)
```

练习：把之前的线程创建的代码修改一下，修改abc函数，在函数内部添加pthread_exit退出的函数，在main函数中，设置打印5次world之后，取消abc这个线程，自己测试能否被取消，若被取消，接着再在abc函数中设置自己的属性为不能被取消，再测试代码

#### 1.3等待一个线程的释放

```c
#include <pthread.h>

int pthread_join(pthread_t thread, void **retval);
@thread:需要等待的那个线程id
@retval：可以用来获取线程函数的返回值，不需要获取添NULL
Compile and link with -pthread.
	线程：
		int *a = malloc(sizeof(int));//0x3000
		*a = 1;
		pthread_exit((void*)a);//只能放在线程中(函数)使用
	main
		int *exit_code;
		pthread_join(thread, (void **)&exit_code);
		printf("exit code = %d\n",*exit_code);
		
	//pthread_join函数的内部
	pthread_join(pthread_t thread, void **retval)//retval = &exit_code
	{
		*retval = a;//*retval = *&exit_code = exit_code = a  => exit_code = 0x3000
	}
pthread_join作用：
	1.等待线程的退出
	2.回收被等待的线程的资源
    
线程退出不代表所有的资源被释放，这取决于线程的属性(detach state分离属性)
    分离属性：
    	在线程退出后，资源会自动释放
    	其他的去调用pthread_join的函数，只会有等待线程退出的作用
    非分离属性(默认属性)
     	在线程退出后，资源不会退出后就完全释放
    	其他的去调用pthread_join的函数，还会有回收资源  	
```

```c
#include <pthread.h>

int pthread_detach(pthread_t thread);
thread:指定要设置分离属性的那个线程
Compile and link with -pthread.

pthread_self:获取自己的线程id
eg:
	设置自己为分离属性，那么在退出线程后，自动释放资源
	pthread_detach(pthread_self());
```

# 三、网络编程

## 1、网络编程基础

### 1.网络通信

通信双方，需要满足两个必须的条件，才能通信

1. 介质/物理媒介(物理层面)

   eg:空气，网线，光纤，电磁波

2. 协议(软件层面)

   物理媒介只保证网络数据正常的接收，但是只是接收还不行，还必须理解其中的含义

   通信双方需要实现约定好通信的协议

   - 协议

     从应用的角度，协议可以理解为规则，是数据传输和解释的规则

     假设 A B双方传输文件，可以约定以下规则

     ```
     第一次，传输文件名
     第二次，发送文件大小
     第三次，传输文件内容
     ```

   - 典型协议

     - 传输层

       ```c
       TCP：传输控制协议，是一种面向连接的，可靠的，基于字节流的传输层通信协议
       UDP：用户数据报，无连接的，不可靠的
       ```

     - 应用层

       ```
       微信，qq,feiq,ftp(文件传输协议),http(超文本传输协议),DNS域名解析协议.....
       ```

     - 网络层

       ```
       ip:因特网互联协议
       ICMP:因特网控制报文协议(TCP/IP)协议簇的子协议，用于ip主机和路由器之间传递控制消息
       IGMP:因特网组管理协议，是一个组播协议，运行在主机和组播路由器之间
       ```

     - 网络接口层

       ```
       arp 正向地址解析解析 ip->mac
       arap 反向地址解析解析  mac->ip
       ```

### 2.网络协议层次模型

1.层次：把不同的功能封装成不同的模块

- 为什么要分层？

  大多数的网络采用分层的体系结果，每一层都建立与它的下层之上，向上一层提供一定的服务，而且把怎么实现这个服务的过程的细节对上一层屏蔽



<img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\01-网络编程基础\网络编程基础.assets\image-20260424110625010.png" alt="image-20260424110625010" style="zoom: 50%;" />

### 3.OSI(open system interconnection)参考模型

开放式系统互联模型

1. 应用层：为程序提供服务

   让你的系统知道你的数据包 是==从哪个应用==发送出去的

2. 表示层： ==数据格式的转化以及数据的加密和解密==

   确保一个系统的应用程序发送的数据 能够 被另外一个系统的应用程序识别

3. 会话层

   ==建立、管理或断开==与应用程序之间的通信，组织和协调两个会话之间的管理和通信

   ```c
   快递公司的调度员，管理者需要管理快递信息(快递何时发送过去，需要多少时间)
   也需要做同步的工作，就好比我把东西运过去了，但是那边没有接收
   ```

4. 传输层(单位：数据报文)

   负责提供 端到端 的传输(同一台设备一般会有多个网络应用程序，用端口号去区号)

   这一层目的确保数据可靠的，有序的传输

   ```
   tcp:提供高可靠的通信：数据无误，数据无丢失，数据无时序，数据无重复到达！！！
   	面向连接：再传输之前先建立连接，确保通信双方都"在线"
   	高可靠通信：建立连接(三次握手) 断开连接(四次挥手) 重发机制 ...
   	
   	对传输质量要求较高，以及传输大量通信数据的时候
   	qq等即使通信软件的用户登录账户相关的功能
   	
   udp
   	不面向连接，不可靠
   	直播，实时应用....
   	包(占的字节会更小)，通信效率更高
   ```

5. 网络层(单位：组)

   ```
   WAN广域网
   LAN局域网
   
   一般通信的主机不是直接连接的，但是在物理方面能够连通，而是通过多个个中间节点(路由器)连接
   ```

   负责将数据传输到目标地址，这一层主要 负责寻址和路由选择(路由器)

   ![image-20260424144404816](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\01-网络编程基础\网络编程基础.assets\image-20260424144404816.png)

   P106

6. 数据链路层(单位：帧)

   负责数据包的封帧即mac寻址

   公网到达局域网之后需要转化为mac地址

   交换机解析判断数据要发送给mac地址对应的哪台电脑

   ```
   arp ip转化mac地址
   arap mac转化ip地址
   ```

   <img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\01-网络编程基础\网络编程基础.assets\image-20260424145143921.png" alt="image-20260424145143921" style="zoom:50%;" />

7. 物理层(单位：bit)

   负责数据在物理网络中的实际传输

   通过光信号和电信号传输

   网卡(每个网卡在出厂的时候，就会设定好一个世界上唯一的mac地址，48bits的数据)，光纤，网线

### 4.常见的网络设备

交换机(swicth),网桥:可以提供大量的网络接口将多个网络连接成局域网

```
工作在数据链路层
通过mac地址来识别网络中的设备，并且根据mac地址转发数据

提高网络效率，减少冲突域，提供网络性能

冲突域：
在同一个网络中，所有的设备共享同一个传输介质，如果有两台或更多的设备同时发送数据，它们的信号就会相互碰撞，产生冲突，导致数据有损坏，所有的设备都需要重新发送
你可以把他想象成一条单车道的小桥
如果只有一辆车通过，很通畅
如果两台车同时从两头开，就会在中间碰撞(冲突)
```

集线器

```
工作在物理层，只是电信号的放大
对接受的信号进行整体放大，扩大网络传输的距离
不识别mac地址和ip地址，会产生冲突域

集线器广播数据，效率低，交换机定向转发数据，效率高
```

路由器

```
工作在网络层，实现路由先择和数据包的转发和交换
```

网关

```
连接两个或两个以上的网络，就是一个网络链接另外一个网络的关口
充当了网络间不同协议之间的转换和翻译的功能
```

调制解调器

```c
信号的转换：将数字信号转化模拟信号(调制)，将模拟信号转化为数字信号(解调)
光猫
```

### 5.网络数据的传输过程

![image-20260424161035224](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\01-网络编程基础\网络编程基础.assets\image-20260424161035224.png)

### 6.互联网ip

ip：网际协议地址

常见的ip地址：

-   IPv4->32bits   2^32个
-   ipv6->128bits  2^128个

公网ip和私有ip有什么区别？

>   公网ip一般由运营商分配的，只有公网ip才能上网

ipv4地址有32bits,分成两个部分：网段号，主机号

>   网段号：用来表示某一个网段，在ip地址是连续的高位
>
>   主机号：用来标识特定网段的特定的主机，再ip地址中是连续的低位
>
>   ipv4
>
>   ```
>   采用的方式 点分十进制->ip字符串
>   
>   eg:
>   	ipv4点分二进制	11000000	10101000	00000001	00000001
>   	ipv4点分十进制	192			168			1			1
>   		"192.168.1.1"
>   网段号 + 主机号 = 32位
>   ```
>
>   

在设置ip的时候，一般需要设置子网掩码netmask

>   ip地址的网段号和主机号是根据子网掩码划分
>
>   子网掩码和ip地址位数是一致，子网掩码中为1的bit数表示网络号的位数，子网掩码中为0的bit数表示主机号的位数，
>
>   ```c
>   eg:
>   	ipv4 :	172.50.0.1         10101100 00110010 00000000 00000001
>   	netmask: 255.255.254.0  => 11111111 11111111 11111110 00000000
>   
>   	网段号(高23位) 主机号(低9位)
>   	理论上 可以分配多少台主机的ip  2^9
>   
>   	广播地址：10101100 00110010 00000001 11111111
>   			 172	  50       1        255  
>   	网段号：  10101100 00110010 00000000 00000000
>   			 172	  50       0        0
>   	默认网关：10101100 00110010 00000001 00000001
>   			 172	  50       1        1
>   ```
>
>   广播地址：ip地址中主机号每个bit都为1的
>
>   网段号：ip地址，主机号全为 0 指定某个网络
>
>   默认网关：一般是路由器的默认网关，ip地址主机号为1

同一网络中子网掩码是相同的，网段号也是一样

>   练习：
>
>   1.子网掩码为255.255.254.0，请问192.168.0.4和192.168.0.3是不是在同一网段？
>
>   是
>
>   2.子网掩码为255.255.254.0，请问192.168.0.4和192.168.1.5是不是在同一网段？
>
>   是
>
>   3.已知当前网络中的某一台主机ip:192.168.31.156,子网掩码为255.255.255.0，求该网络的默认网关和广播地址，以及实际能分配ip给多少台主机
>   默认网关：192.168.31.1
>
>   广播地址：192.168.31.255
>
>   分配主机：2^8 -2 =254

### 7.端口号

### 8.网络字节序

ip地址转换

```
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int inet_aton(const char *cp, struct in_addr *inp);

in_addr_t inet_addr(const char *cp);

in_addr_t inet_network(const char *cp);

char *inet_ntoa(struct in_addr in);
```

端口号转换

```
#include <arpa/inet.h>

uint32_t htonl(uint32_t hostlong);

uint16_t htons(uint16_t hostshort);

uint32_t ntohl(uint32_t netlong);

uint16_t ntohs(uint16_t netshort);
```

## 2、socket网络套接字

socket是一个编程接口，作用就是实现网络上不同主机的应用程序进程双向通信

套接字当成是一种特殊的文件描述符，也就意味者我们使用套接字实现网络通信使用read/write

socket的类型

1. 流式套接字(SOCK_STREAM)

   ```
   流式套接字用于提供面向连接，可靠的数据传输服务
   主要针对传输层协议为TCP协议的应用
   ```

2. 数据报套接字(SOCK_DGRAM)

   ```
   数据报套接字用于提供一种无连接的服务(不能保证数据传输的可靠性)
   主要针对传输层协议为UDP协议的应用
   ```

客户端和服务器模型

>   任何网络应用都有通信双方
>
>   -   客户端或用户端(client),是为服务端提供本地服务的程序或硬件设备
>   -   服务端(server)就是指在网络中为客户端提供某些服务的计算机系统
>
>   服务器与客户端一对多的关系

### 1.tcp套接字编程流程

![image-20260427113420437](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260427113420437.png)

<img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260427113507565.png" alt="image-20260427113507565" style="zoom:67%;" />

### 2.具体的API接口函数

1.socket:创建一个套接字

```c
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
@domain：指定域或协议簇
	socket接口不仅仅局限于TCP/IP,它还有蓝牙协议，本地通信协议...
	AF_INET  ->IPV4
	AF_INET6 ->IPV6
	AF_UNIX/AF_LOCAL  ->unix域协议 本地进程通信
@type:套接字类型
	SOCK_STREAM -> TCP
	SOCK_DGRAM	-> UDP
@protocol:指定应用层协议，一般指定为0(不知名的私有协议)
返回值：
	成功 返回一个套接字的文件描述符
	失败 -1
eg：
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```

2.bind：绑定一个主机的网络地址(服务器)

```c
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr,
socklen_t addrlen);
@sockfd：要绑定地址的套接字描述符
@addr:通用的网络地址结构体的指针
@addrlen:第二个参数执行的地址结构体长度
返回值：
	成功 0
	失败 -1
eg:
	#include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;//ipv4
	sa.sin_port = htons(8888);//指定端口号，把端口号主机字节序转成网络字节序
	inet_aton("172.50.1.11", &sa.sin_addr);//指定ip 将点分十进制的ip地址转成二进制网络ip
	/*
		监听发送给172.50.1.11的数据包
		还可以写127.0.0.1 -> 同一台主机通信
		还可写0.0.0.0  -> 服务器绑定ip更推荐用这个
		表示监听服务器主机上所有的ipv4的地址，也就是说
		无论是从哪个ip地址发送和过来，只要是目的端口和监听的端口相匹配的
		服务器都可以处理数据包
	*/
	if(bind(sockfd, (struct sockaddr *)&sa,sizeof(sa)) == -1)
	{
		perror("bind failed");
		return -1;
	}
```

网络地址结构体

>   socket接口不仅用于ipv4,IPv6...蓝牙，本地协议....不同的协议的地址的类型是不一样的
>
>   通用的网络地址结构体
>
>   ```c
>   struct sockaddr {
>      sa_family_t sa_family;
>      	//指定协议簇 eg:ipv4
>      char        sa_data[14];
>      	//包含套接字中的目的地址和端口地址
>   }
>   这个结构体的缺陷就是把ip地址和端口混在一起保存到sa_data里面
>   ```
>
>   ipv4专用结构体:man 7 ip
>
>   ```c
>   #include <sys/socket.h>
>   #include <netinet/in.h>
>   #include <netinet/ip.h> /* superset of previous */
>   
>   struct sockaddr_in {
>      sa_family_t    sin_family; /* address family: AF_INET */
>      	//指定协议簇 eg:ipv4
>      in_port_t      sin_port;   /* port in network byte order */
>      	//指定端口号 eg:8888 9999   需要把主机字节序转化为网络字节序
>      struct in_addr sin_addr;   /* internet address */
>      	//指定ip地址 eg:172.50.1.11  还需要将点分十进制ip地址 转换 二进制网络字节序的ip
>   };
>   
>   /* Internet address. */
>   struct in_addr {
>      uint32_t       s_addr;     /* address in network byte order */
>   };
>   ```
>
>   

3.listen:让套接字进入监听模式

```c
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int listen(int sockfd, int backlog);
@sockfd:要监听的套接字
@backlog：同时能够处理连接请求
	eg:5,10....
返回值：
	成功 0
	失败 -1

eg:
    //监听
    if(listen(sockfd,5) == -1)
    {
        perror("listen failed");
        return -1;
    }
```

4.accept：用于server接收一个来自客户端的连接请求

```c
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
@sockfd:要监听的套接字
@addr:用来保存客户端的网络通信地址
	如果不需要可以填NULL
	如果需要
		struct sockaddr_in client_addr;//使用(struct sockaddr *)&client_addr
@addrlen：用来保存客户端网络通信地址字节长度
	如果不需要可以填NULL
	如果需要
		socklen_t client_addrlen = sizeof(struct sockaddr_in);//使用&client_addrlen
返回值：
	成功返回与该客户端连接的套接字描述符，后续与该客户端通信都是用这个套接字描述符
    失败 -1
eg:不想知道对方的网络地址
	int newclientfd = accept(sockfd, NULL, NULL);
eg：想知道对方的网络地址
	struct sockaddr_in client_addr;//保存对方的网络地址
	socklen_t client_addrlen = sizeof(struct sockaddr_in);//保存对方网络地址大小
	int newclientfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
	//打印对象的ip地址和端口号
	printf("%s[%d]已成功连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
```

5.connect:客户端连接服务器

```c
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr,
socklen_t addrlen);
@sockfd：要绑定地址的套接字描述符
@addr:通用的网络地址结构体的指针
@addrlen:第二个参数执行的地址结构体长度
返回值：
	成功 0
	失败 -1
类比于bind函数
```

练习：

```
server.c
	socket
	bind
	listen
	accept
	
	read(newclientfd,buf,sizeof(buf));
	close
client.c
	socket
	connect
	write(sockfd,"xxxx",strlen("xxxx"));
	close
```

6.往套接字上面发送数据

write/send/sendto  tcp三个都可以用 但是udp只能用sendto

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t send(int sockfd, const void *buf, size_t len, int flags);
前面三个参数跟write一模一样的，你write写什么你的send就写什么
@flags:标记，一般填0(阻塞)
		MSG_DONTWAIT非阻塞

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
const struct sockaddr *dest_addr, socklen_t addrlen);
前面四个参数跟send一模一样的，你send写什么你的sendto就写什么
@dest_addr:指定接收放的网络通信地址
	如果是tcp通信，这个参数可以省略，填NULL
	如果是udp通信,需要指定
@addrlen:对方地址的长度
	如果是tcp通信，addrlen=0
	如果是udp通信,需要指定

对于第5个参数和第6个参数，connect最后两个参数怎么写你就怎么写
返回值：
	返回实际发送的字节数
	失败 -1
	
eg:
	struct sockaddr_in sa;
	sa.sin_family = AF_INET;//ipv4
	sa.sin_port = htons(8888);//指定端口号，把端口号主机字节序转成网络字节序
	inet_aton("172.50.1.11", &sa.sin_addr);//指定ip 将点分十进制的ip地址转成二进制网络ip
	sendto(sockfd,"xxxxx",strlen("xxxxx"),0,(struct sockaddr *)&sa,sizeof(sa));
```

7.接收套接字上面的数据

read/recv/recvfrom  tcp三个都可以用 但是udp只能用recvfrom

```c
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recv(int sockfd, void *buf, size_t len, int flags);
前面三个参数跟read一模一样的，你read写什么你的recv就写什么
@flags:标记，一般填0(阻塞)
		MSG_DONTWAIT非阻塞
		
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
struct sockaddr *src_addr, socklen_t *addrlen);
前面四个参数跟recv一模一样的，你recv写什么你的recvfrom就写什么
@src_addr:指定保存发送方的网络通信地址
	如果是tcp通信，这个参数可以省略，填NULL
	如果是udp通信,需要指定
@addrlen:发送方地址的长度
	如果是tcp通信，addrlen=0
	如果是udp通信,需要指定

对于第5个参数和第6个参数，accept最后两个参数怎么写你就怎么写
返回值：
	返回实际接收的字节数
	失败 -1
	
eg:
	char buf[100] = {0};
	struct sockaddr_in client_addr;//保存对方的网络地址
	socklen_t client_addrlen = sizeof(struct sockaddr_in);//保存对方网络地址大小
	recvfrom(sockfd,buf,sizeof(buf)-1,0,(struct sockaddr *)&client_addr, &client_addrlen);
```

练习：利用tcp写一个网络应用程序
		服务器：用来循环接受(recv)数据
		客户端：用来循环发送(send)数据(从键盘获取数据再发送)，如果发送的数据为q,退出循环，服务器也退出

思考：怎么实现一对多通信，客户端(循环发送数据)代码不用改，只改服务器代码(循环接受数据)，写线程版代码，思考哪个位置开线程合适，那么线程函数中该做什么事情

```
主线程
	socket
	bind
	listen
	while(1)
	{
		accept返回一个专门跟客户端通信的fd,就开辟一个线程，专门用于与这个客户端通信
		//创建线程
	}
子线程
	接收数据(思考怎么拿到newclientfd)
```

```
socket
bind
listen
while(1)
{
    accept
    fork();
    //子进程循环接收数据 
}
```

作业：利用tcp写一个网络传输文件的程序

```
客户端:发送文件
服务器:接收文件

file_tcp_client.c
int send_file()
{
	//发送文件名的长度大小
	//发送文件名
	
	//发送文件大小
	
	//发送文件内容
	打开文件
	while(1)//循环到什么时候结束，读到文件末尾结束
	{
		读取文件内容
		再把读到的内容发送给服务器
	}
	关闭文件
}

file_tcp_server.c
int recv_file()
{
	//切换工作路径，不能与传输的文件在同一路径下
	
	//接收文件名的长度大小
	//接收文件名
	
	//接收文件大小
	
	//接收文件内容
	打开文件
	while(1)//循环到什么时候结束，接收的总字节数 等于 接收的文件大小
	{
		读取来自客户端的数据
		写入到文件
	}
	关闭文件
}
```

### 3.三次握手

建立连接

三次握手情景模拟

```
客户端						服务端
你好，我希望建立连接
						好的，我已经准备好了，可以建立连接
好的，马上连接
=>建立连接成功
```

为什么需要三次握手？

```
双方能够连接到一起的条件是什么？
	必须满足双方都能收和发
	第一次 客户端能不能发送
	第二次 服务器能不能接收和发送
	第三次 客户端能不能接收
```

三次握手具体过程

>   tcp数据包的结构
>
>   <img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260428100432695.png" alt="image-20260428100432695" style="zoom: 67%;" />
>
>   1.序列号：seq占32位，用来表示从计算机A发送到计算机B的数据包的编号
>
>   2.确认序列号：ack占32位，客户端和服务器可以发送 ack = seq + 1 （我收到了你发的seq这个包，我期待下一次收到seq+1的包）
>
>   3.标志位：每个标志位占1bit,共有6个分别为：URG,ACK,PSH,RST,SYN,FIN
>
>   -   ACK:确认序列号有效
>   -   RST:重置连接
>   -   SYN：建立一个新的连接
>   -   FIN：断开连接
>
>   <img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260428101239689.png" alt="image-20260428101239689" style="zoom: 67%;" />
>
>   ```c
>   第一次握手：
>   	客户端随机生成一个序列号x,同时把SYN置1，标识这是SYN报文(同步报文)
>   	接着把同步报文发送给服务端，表示向服务器发起连接请求，该报文中不包含应用层数据
>   	之后，客户端处于SYN_SENT
>   
>   第二次握手：
>   	服务器收到客户端syn包，也随机生成一个序列号y,设置确认应答号为x+1
>   	然后把SYN和ACK的标志置1
>   	最后把报文发送给客户端，该报文中不含应用层数据
>   	服务器处于SYN_REVD
>   
>   第三次握手：客户端收到服务端的报文后，回一个应答的报文
>   	应答的报文中，序列号x+1,确认应答号设置y+1,然后把ACK标志置1
>   	之后，客户端和服务器进入ESTAB-LISHED
>   	建立连接成功
>   ```
>
>   

### 4.四次挥手

断开连接，它让计算机释放不再使用的资源

```
客户端					服务器
下班了，走，打球去
					好的，我换个鞋子，稍等下一个
					....
					换好了，走
好
=>断开连接成功了
```

<img src="C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260428105909098.png" alt="image-20260428105909098" style="zoom:67%;" />

```c
一开始 双方处于ESTAB-LISHED
通信过程中，双方都可以主动断开连接，多数是客户端主动关闭，以客户端断开为例子：
1.客户端希望断开连接，就给服务器发送一个标志FIN为1的报文，客户端进入FIN-WAIT-1
2.服务器收到FIN的报文，回一个ACK标志为1的应答报文，服务端进入到CLOSE-WAIT,客户端收到ACK的报文，进入到FIN-WAIT-2
3.如果服务端也希望断开连接，就给客户端发送一个标志为FIN和ACK标志为1的报文，之后服务端进入LAST-ACK
4.客户端收到服务端的FIN的报文，回一个ACK标志为1的报文，之后客户端进入TIME-WAIT
```

```
注意：
	主动断开的一方，四次挥手之后，会进入到TIME-WAIT
	TIME-WAIT 持续2MSL
	MSL:表示一个报文最大的生存时间，在linux下，一个MSL默认30s
```

解决bind failed: Address already in use的问题:设置套接字选项
![image-20260428111824373](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\02-socket网络套接字\socket网络套接字.assets\image-20260428111824373.png)

```
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname,
const void *optval, socklen_t optlen);
@sockfd
@level：要设置的选项的那个级别 -> 查表
@optname:要设置的那个选项的名字(宏) -> 查表
@optval：要设置的选项的值
@optlen：选项值的长度

int n = 1;
setsockopt(sockfd, SOL_SOCKET,SO_REUSEPORT,(void *)&n, (socklen_t)sizeof(n));//允许端口复用
setsockopt(sockfd, SOL_SOCKET,SO_REUSEADDR,(void *)&n, (socklen_t)sizeof(n));//允许地址复用

放在bind之前
```

### 5.UDP

## 3、Unix域协议

### 1.unix域协议

利用socket编程接口实现 本地进程间通信

unix域协议：AF_UNIX 或 AF_LOCAL 

unix域协议提供了两套套接字

-   SOCK_STREAM --- TCP 面向字节流
-   SOCK_DGRAM --- UDP面向数据报

unix域协议的特点：

1.  与TCP/UDP相比，速度更快，数据不需要传递到主机外，不需要进行封包和拆包的过程
2.  相对于IP协议来说。IP协议是根据IP地址+端口号确定进程，而unix域协议使用还是文件系统中路径名

unix域协议大部分的编程流程和函数和TCP或UDP类似的，只不过unix域协议使用路径文件名描述一个地址

unix域协议的地址结构体类型：man 7 unix

```
#include <sys/socket.h>
#include <sys/un.h>
struct sockaddr_un {
    sa_family_t sun_family;               /* AF_UNIX */
    	//AF_UNIX或AF_LOCAL
    char        sun_path[108];            /* pathname */
    	//unix域协议地址，是'\0'结尾的本地文件系统的绝对路径名，eg:"/home/china/xxx.socket"
};
```

unix字节流套接字编程流程(类似于TCP)

```
server:socket -> bind -> listen -> accept read/recv/recvfrom -> close
client:socket -> connect -> write/send/sendto -> close
```

unix数据报套接字编程流程(类似于UDP)

```
server:socket -> bind -> recvfrom -> close
client:socket -> sendto -> close
```

练习：利用unix域协议 写一个程序 实现服务器接收数据，客户端发送数据(以TCP为例子)

```
server.c
#define unix_path "/home/china/unix2612.socket"
int main()
{
	//删除
	unlink(unix_path);
	
}
```

```c
a.假设有多个客户端连接到同一个服务器， 服务器可以保存多个连接的客户端的信息（对方IP 以及 端口
号，记录在线状态）
b.某个客户端可以实现给指定某一个客户端发消息（列出当前在线列表(服务器将在线列表数据发送给当前客户端)-- 选择一个用户 -- 发送消息）

列表显示：
编号 ip	   		port	  在线状态
1 	172.50.1.22  28283      1
2 	172.50.1.23  28233	    1
3 	172.50.1.88  32128      1

输入格式为pm 1 nihao     //pm表示为私聊功能 发送给编号为1的用户 nihao的消息
输入格式为list      	      //list表示为获取当前在线用户功能
```

## 4、IO多路复用

IO模型：输入输出模型

我们知道linux中输入输出不是直接从键盘获取数据，也不是直接往终端输出(缓冲区)

那么我们可以把IO理解为两个步骤：

1.  等待IO事件的就绪。比如：读，我们要等待缓冲区有数据可读  写，需要等待缓冲区有空间可写
2.  真正意义上的数据的迁移(涉及到用户态和内核态的切换) 

### 1.linux中五种IO模型

#### 1.阻塞模型

![image-20260428163916922](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260428163916922.png)

缺点：如果是在单线程环境下，由于是阻塞地获取结果，只能有一个客户端连接。如果是多线程环境下，需要不断地新建线程来接收客户端。这样会浪费大量的空间和资源。

#### 2.非阻塞模型

意味着程序无需等待结果，持续运行

- 读：如果有数据，则读取数据

  ​		如果没有数据，就立即返回

- 写：如果有空间，则直接写入

  ​		如果没有空间，即立即返回

![image-20260428163937199](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260428163937199.png)

缺点：采用轮询，占用cpu, 不及时

虽然这种方式不会阻塞，但是有一个缺点，这种方式相对于阻塞IO来说，效率更低，会浪费大量的cpu的时间

#### 3.IO多路复用

- 允许同时对多个IO事件进行控制，同时在监控多个"文件描述符"

  同时监听多个文件描述符是否就绪(是否可写/可读/出错)

- IO多路复用实现的机制是通过select/poll/epoll函数来实现

  先有select,再有了poll/epoll,由两个不同的组织写的，但是作用是类似

  ![image-20260428164005362](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260428164005362.png)

#### 4.信号驱动IO

如果有IO事件就绪，就可以发送一个信号给应用程序，进行数据的处理

就相当于注册一个信号处理函数，当数据准备好了，就给我发送一个信号，我捕捉这个信号之后，就可以直接去信号处理函数里面去获取数据即可![image-20260428164044316](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260428164044316.png)

#### 5.异步IO

相当于E去钓鱼，但是E还有其他的事情要做，所有它打电话叫来了它的好朋友F帮他看钩子，一旦鱼上钩了，F就会起钩， 然后打电话告诉E鱼钓上来了

进程E仅仅就是对数据的请求，而数据的监控和数据的迁移都是由F完成的

![image-20260428164108668](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260428164108668.png)

### 2.IO多路复用的实现方式

什么是多路复用？

就是预先把要监听的文件描述符加入到集合，然后在规定的时间内或无限时间内进行等待。如果在规定的时间内，集合中文件描述符没有数据的变化，则说明超时接收，并且进行下一次规定时间再等待。一旦集合中文件描述符有数据变化，则其他的没有数据变化的文件描述符就会被剔除到集合外，并且再次进入下一次等待状态

select原理如图所示：

![image-20260429101152147](C:\Users\Zelda\Desktop\Rk3568_Code\3_网络编程\03-网络编程\01-note\04-IO多路复用\IO多路复用.assets\image-20260429101152147.png)

特点：可以实现多个文件描述符进行监听。一般在网络服务器中，可以并发的处理多个客户端的连接和请求

并发：创建进程/线程/多路复用

#### 1.select

```c
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

typedef /* ... */ fd_set;

int select(int nfds, fd_set *readfds,
fd_set *writefds,
fd_set *exceptfds,
struct timeval *timeout);
@nfds:三个集中中最大的文件描述符+1,因为这个参数会告诉内核检测前多少个文件描述符
@readfds：可读文件描述符集合
@writefds：可写文件描述符集合
@exceptfds：错误文件描述符集合
@timeout：设置阻塞等待的时间，有3种情况
		1.设置NULL，永远等待，这个函数阻塞的(无限等待，直到有文件描述符发送变化)
		2.设置timeout,等待一个固定的时间
			struct timeval
			{
				long tv_sec;//秒
				long tv_usec;//微妙
			};
			eg：等待5s
				struct timeval tv;
				tv.tv_sec = 5;
				tv.tv_usec = 0;
			在这段时间中，若select正常返回，timeout的值，被设置成剩余的秒数
			若想每次调用select都等待固定的时间，那么需要再调用前重新设置timeout
		3.设置0 非阻塞
返回值：
	>0 表示已经就绪的文件描述符的个数
		FD_ISSET去判断到底是哪个文件描述符就绪
	=0	超时
	<0  出错

void FD_CLR(int fd, fd_set *set);//把fd指定的文件从set指定的集合中移除
int  FD_ISSET(int fd, fd_set *set);//判断fd是否就绪
void FD_SET(int fd, fd_set *set);//把fd加入到指定的集合中
void FD_ZERO(fd_set *set);//把set指定的集合清空
```

eg：tcp服务器代码步骤

```c
//创建套接字  sockfd
//绑定
//监听
int clientfds[50] = {0};//保存所有的客户端的fd
int client_count = 0;//客户端的数量
int maxfd = sockfd;//初始化最大的文件描述符
while(1)
{
	//1.定义一个集合
	fd_set readfds;//可读文件描述符集合
    //2.清空集合
    FD_ZERO(&readfds);
    //3.把你需要要监听的fd加入到可读集合
    FD_SET(sockfd, &readfds);//添加服务器的fd
    for(int i = 0;i<client_count;i++)
    {
    	FD_SET(clientfds[i], &readfds);//添加全部客户端的fd
    }
    //4.设置超时时间 5s
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    //5.select
    int ret = select(maxfd+1,&readfds,NULL,NULL,&tv);
    //6.处理结果
    if(ret > 0)//检查具体的fd
    {
    	if(FD_ISSET(sockfd,&readfds))//sockfd是socket的返回值 sockfd就绪
    	{
    		//代表有客户端想你发起连接
    		int newclientfd = accept(.....);
    		clientfds[client_count] = newclientfd;
    		client_count++;//客户端的数量+1
    		if(maxfd < newclientfd)
    		{
    			//更新maxfd
    		}
    	}
    	for(int i = 0;i<client_count;i++)
    	{
    		if(FD_ISSET(clientfds[i],&readfds))//判断clientfds[i]是否就绪
    		{
    			//有客户端给你发送数据 read/recv/recvfrom
    			//read/recv/recvfrom 返回值0 代表有客户端退出 把clientfds[i]在clientfds数组中删除
    		}
    	}
    }
    else if(ret == 0)
    {
    	printf("超时\n");
    }
    
}
```

练习：利用select实现tcp一对多的通信(只改服务器的代码)

#### 2.poll

```c
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
@fds：结构体指针，指向要监听的结构体数组
    struct pollfd {
        int   fd;         /* file descriptor */
        short events;     /* requested events */要监听的事件
        	/*
        		POLLIN	可读
        		POLLOUT	可写
        		POLLERR	出错
        		...
        	*/
        	eg:
        		可读可写：POLLIN|POLLOUT
        short revents;    /* returned events */返回已经就绪的事件
        	eg:判断是否可读
        		struct pollfd fd;
        		if(fd.revents & POLLIN)
        		{
        			//fd的读就绪
        		}
    };
@nfds:表示上面的那个数组的元素的个数
@timeout:超时时间 ms为单位
返回值：
	>0 就绪文件的个数
		由于你监听了多个文件描述符，到底是哪一个文件描述符就绪，使用revents一个一个判断
	=0 超时
	<0 出错
```

```C
//创建套接字  sockfd
//绑定
//监听
int clientfds[50] = {0};//保存所有的客户端的fd
int client_count = 0;//客户端的数量
while(1)
{
	//1.定义一个struct pollfd类型的数组，相当于是监听集合
	struct pollfd fds[10];
    //2.把你需要监听的fd加入到监听集合
    //添加服务器的fd
    fds[0].fd = sockfd;
    fds[0].events = POLLIN;//可读
    fds[0].revents = 0;//初始还没有就绪
    //添加全部客户端的fd
    for(int i = 0;i<client_count;i++)
    {
    	fds[i+1].fd = clientfds[i];
        fds[i+1].events = POLLIN;//可读
        fds[i+1].revents = 0;//初始还没有就绪
    }
    //3.poll
    int ret = poll(fds,client_count+1,5000);
    //6.处理结果
    if(ret > 0)//检查具体的fd
    {
    	if(fds[0].revents & POLLIN)//sockfd是socket的返回值 sockfd就绪
    	{
    		//代表有客户端想你发起连接
    		int newclientfd = accept(.....);
    		clientfds[client_count] = newclientfd;
    		client_count++;//客户端的数量+1
    	}
    	for(int i = 0;i<client_count;i++)
    	{
    		if(fds[i+1].revents & POLLIN)//判断clientfds[i]是否就绪
    		{
    			//有客户端给你发送数据 read/recv/recvfrom
    			//read/recv/recvfrom 返回值0 代表有客户端退出 把clientfds[i]在clientfds数组中删除
    		}
    	}
    }
    else if(ret == 0)
    {
    	printf("超时\n");
    }
    
}
```

练习：利用poll实现tcp一对多的通信(只改服务器的代码)

#### 3.epoll

epoll = 红黑树维护监听集合 + 回调通知机制 + 就绪队列直接读取

=> 避免了线性扫描，实现O(1)

映射机制(核心)

```
用户态fd
	↓红黑树快速查找
内核 strct file*(socket的结构体)
	↓事件发生
回调ep_poll_callback
	↓
把对应的结点插入到就绪队列中
```

1. epoll_create

   创建一个epoll对象(返回一个epfd,当成是监听集合)

   ```
   1.申请eventpoll结构体(epoll集合)
   2.epfd包含 红黑树的根+就绪队列的对头
   ```

   ```
   #include <sys/epoll.h>
   
   int epoll_create(int size);
   @size:填大于0的数即可，该参数已经被忽略掉了
   返回值：
   	成功 返回一个epoll的对象 对象就是一个文件描述符(监听集合)
   ```

2. epoll_ctl

   添加/修改/删除

   ```
   1.把fd对应的结点插入到红黑树
   2.把ep_poll_callback注册到socket的等待队列当中
   ```

   ```C
   #include <sys/epoll.h>
   
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
   @epfd:填上面epoll_create的返回值
   @op：你的操作
   	EPOLL_CTL_ADD
   	EPOLL_CTL_MOD
   	EPOLL_CTL_DEL
   @fd：你要操作的文件描述符
   @event：要监听的时间的结构指针
       typedef union epoll_data {
           void        *ptr;
           int          fd;
           uint32_t     u32;
           uint64_t     u64;
       } epoll_data_t;
   
       struct epoll_event {
           uint32_t     events;      /* Epoll events */
           	/*
           		EPOLLIN
           		EPOLLOUT
           		EPOLLERR
           		EPOLLET 边沿触发 -> 优势
           			ET 边沿触发
           			LT 水平触发/级别 默认
           	*/
           epoll_data_t data;        /* User data variable */
       };
   返回值：
   	成功 0
   	失败 -1
   1.LT水平触发模式下，只要这个fd还有数据可读，每次epoll_wait都会返回它的事件，提醒用户程序去操作
   2.ET边沿触发模式下，它只会提示一次，直到下次再有数据流入之前都不会提示
   	eg:
   		1.读缓冲区开始时候是空的
   		2.读缓冲区写入2kb的数据
   		3.水平触发和边沿触发此时收到可读的信号
   		4.收到信号通知后，读取1kb的数据，读缓冲区剩下1kb的数据
   		5.水平触发会再次通知，而边沿触发不会再次进行通知
   		
   eg:
   	//添加
   	struct epoll_event ev;
       ev.events = EPOLLIN|EPOLLET;
       ev.data.fd = sockfd;
   	epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
   	
   	//删除
   	epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,NULL);
   ```

   

3. epoll_wait

   ```
   1.检查就绪队列
   2.把就绪事件拷贝到用户空间
   3.清空就绪队列
   ```

   ```c
   #include <sys/epoll.h>
   
   int epoll_wait(int epfd, struct epoll_event *events,
   int maxevents, int timeout);
   @epfd:epoll_create的返回值
   @events：用来放就绪事件的数组
   @maxevents：就绪事件的最大值
   @timeout：超时时间
   返回值：
   	>0 返回就绪事件的个数
   eg:
   	struct epoll_event events[10];
   	int ret = epoll_wait(epfd, events,10,5000);
   	if(events[0].events & EPOLLIN)//就绪
   	{
   		if(events[0].data.fd == sockfd)//sockfd就绪
   		{
   			//accetp
   		}
   	}
   ```

   eg:服务器使用epoll的步骤

   ```
   //创建套接字 sockfd
   //绑定
   //监听
   //(1)创建一个epoll对象(集合) epfd
   
   //(2)要把监听的文件描述符添加到epoll对象
   //添加服务器的文件描述符
   struct epoll_event ev;
   ev.events = EPOLLIN|EPOLLET;
   ev.data.fd = sockfd;
   epoll_ctl(epfd,EPOLL_CTL_ADD,sockfd,&ev);
   
   //(3)等待监听事件的产生
   struct epoll_event events[10];//用来保存就绪事件的数据
   int ret = epoll_wait(epfd, events,10,5000);
   if(ret > 0)
   {
   	for(int i = 0;i<ret;i++)
   	{
           if(events[i].events & EPOLLIN)//就绪
           {
               if(events[i].data.fd == sockfd)//sockfd就绪
               {
                   //accept
                   int newclientfd = accept(...);
                   //添加客户端的文件描述符到监听结合
                   struct epoll_event ev;
                   ev.events = EPOLLIN|EPOLLET;
                   ev.data.fd = newclientfd;
                   epoll_ctl(epfd,EPOLL_CTL_ADD,newclientfd,&ev);
               }
               else
               {
               	//接收客户端的数据 read/recv/recvfrom 
               }
           }
       }
   }
   ```

   练习：利用epoll实现tcp的一对多通信





- json：[JSON](https://www.json.org/) 全称 JavaScript Object Notation，即 JS对象简谱，是一种轻量级的数据格式。它采用完全独立于编程语言的文本格式来存储和表示数据，语法简洁、层次结构清晰，易于人阅读和编写，同时也易于机器解析和生成，有效的提升了网络传输效率。

- cjson:基于C语言的json库，有两个源码文件 cJSON.c 和cJSON.h

  ```
  gcc xxx.c cJSON.c
  ./a.out
  ```

- 语法

  ```c
  json对象是一个无序的“名称/值”键值对的集合
  -以{开始 以}结束,允许嵌套使用
  -每个名称和值成对出现，名称和值之前使用:分隔
  -键值对之间用，分隔
  -这些字符前后允许出现无意义的空白符
  
  对于键值，可以有如下的值
  -一个是新的json的对象
  -数组：使用[和]表示
  -数字：直接表示，可以是整数，也可以是浮点数
  -字符串：使用""表示
  ```

- cjson结构体

  ```c
  /* The cJSON structure: */
  typedef struct cJSON {
  	struct cJSON *next,*prev;	 //next指针：指向下一个键值对  prev指针指向上一个键值对
  	struct cJSON *child;		//在键值对结构体中，当该键值对的值是一个嵌套的JSON数据或者一个数组时，由child指针指向该条新链表。
  
  	int type;					//键值对 中   值的数据类型  cJSON Types
  
  	char *valuestring;			//如果 键值对中的值，是字符串类型，该变量就是字符串的地址
  	int valueint;				//如果 键值对中的值，是int类型，该变量就是值的数据
  	double valuedouble;			//如果 键值对中的值，是double类型，该变量就是值的数据
  
  	char *string;				//存放 键值对 中 的 键（键都是字符串类型）
  } cJSON;
  
  eg:
  	{
  		"name":"lixiang",
  		"age":18,
  		"score":[80,90,100]
  	}
  
  	CJSON*object;//假设让它指向{}
  	object->child;//"name":"lixiang"  里面链表中的第一个结点的地址
  	object->child->string;//name
  	object->child->valuestring;//lixiang
  	object->child->next;//"age":18
  	object->child->next->valueint;//18
  	object->child->next->next;//"score":[80,90,100]
  	object->child->next->next->child;//[80,90,100]指向80这个元素的地址
  	object->child->next->next->child->valueint;//80
  	object->child->next->next->child->next->valueint;//90
  ```



## 5、CJSON

### 1.CJSON数据的封装

1.cjson对象或数组的创建和销毁

```c
//创建一个空值json对象
cJSON * cJSON_CreateNull(void);
//创建一个true值json对象
cJSON * cJSON_CreateTrue(void);
//创建一个false值json对象
cJSON * cJSON_CreateFalse(void);
//创建一个布尔值json对象
cJSON * cJSON_CreateBool(cJSON_bool boolean);
//创建一个整型json对象
cJSON * cJSON_CreateNumber(double num);
//创建一个字符串json对象
cJSON * cJSON_CreateString(const char *string);
//创建一个数组json对象
cJSON * cJSON_CreateArray(void);
//创建一个json对象
cJSON * cJSON_CreateObject(void);
//销毁json对象
void cJSON_Delete(cJSON *item);
```

**2.cjson对象数据添加**

```c
//在对象上添加NULL
cJSON* cJSON_AddNullToObject(cJSON * const object, const char * const name);
//在对象上添加true
cJSON* cJSON_AddTrueToObject(cJSON * const object, const char * const name);
//在对象上添加false
cJSON* cJSON_AddFalseToObject(cJSON * const object, const char * const name);
//在对象上添加布尔值
cJSON* cJSON_AddBoolToObject(cJSON * const object, const char * const name, const cJSON_bool boolean);
//在对象上添加整数
cJSON* cJSON_AddNumberToObject(cJSON * const object, const char * const name, const double number);
//在对象上添加字符串
cJSON* cJSON_AddStringToObject(cJSON * const object, const char * const name, const char * const string);
//在一个对象里面添加另外一个cjson的对象,key叫做name
cJSON*) cJSON_AddObjectToObject(cJSON * const object, const char * const name);
//在一个对象里面添加一个数组，数组的key叫做name
cJSON* cJSON_AddArrayToObject(cJSON * const object, const char * const name);
//在数组json对象array上添加一个item的json对象
cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item);
//在json对象object上添加一个item的json对象，并且key的名字为string
cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item);
```

**3.将cjson格式的对象转化成字符串**

输出JSON数据，可以将整条链表中存放的JSON信息输出到一个字符串中

```c
char * cJSON_Print(const cJSON *item);//格式
char * cJSON_PrintUnformatted(const cJSON *item);//无格式

记得释放空间
```

例子：

```c
{
    "name":"lixiang",
    "age":18,
    "score":[80,90,100]
}
	
#include <stdio.h>
#include "cJSON.h"

int main()
{
    //创建一个cjson对象
    cJSON *cjson_obj =  cJSON_CreateObject();//{}
    cJSON_AddStringToObject(cjson_obj,"name","lixiang");
    cJSON_AddNumberToObject(cjson_obj,"age",18);
    //添加数组到cjson_obj
    cJSON *array = cJSON_AddArrayToObject(cjson_obj, "score");
    //添加元素到数组中
    cJSON_AddItemToArray(array, cJSON_CreateNumber(80));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(90));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(100));
    
    //把cjson转化为字符串
    char *buf = cJSON_Print(cjson_obj);
    printf("%s\n",buf);
    free(buf);

    char *buf2 = cJSON_PrintUnformatted(cjson_obj);
    printf("%s\n",buf2);
    free(buf2);

    //销毁cjson对象
    cJSON_Delete(cjson_obj);//会连同子对象也会一起销毁
}
```

练习：

```c
{
	"name":"luffy",
	"sex":"man",
	"age":19
}
```

```c
["hello world",10,33]
```

```C
{"person":[{"name":"luffy","age":19}]}
```

### 2. CJSON字符串的解析

1.解析json对象

需要删除

```c
//将json字符串转换为 cJSON 结构体
cJSON * cJSON_Parse(const char *value);
```

2.根据键值对的名称从JSON数据中取出对应的值，返回该键值对(链表节点)的地址

```c
//获取对象object中键值为string的json对象
cJSON * cJSON_GetObjectItem(const cJSON * const object, const char * const string);
```

3.如果JSON数据的值是数组，使用下面的两个API提取数据：

```c
//获取数组array下标为index的json对象
cJSON * cJSON_GetArrayItem(const cJSON *array, int index);
//获取数组中元素的个数
int cJSON_GetArraySize(const cJSON *array);
```

4.判断键值类型并打印

```c
cJSON *object;//假设有一个cjson对象object
if(object->type == cJSON_String)//字符串
    printf("%s:%s\n",object->string,object->valuestring);
else if(object->type == cJSON_Number)//数字
    printf("%s:%d\n",object->string,object->valueint);
else if(object->type == cJSON_Array)//数组
{
    printf("%s:",object->string);
    //获取数组中元素的个数
    int num = cJSON_GetArraySize(object);
    for(int i = 0;i<num;i++)
    {
        cJSON *temp = cJSON_GetArrayItem(object,i);//获取数组元素
        if(temp->type == cJSON_String)
            printf("%s ",temp->valuestring);
        else if(temp->type == cJSON_Number)
            printf("%d ",temp->valueint);
    }
    printf("\n");
} 
```

示例

```c
#include <stdio.h>
#include "cJSON.h"

void print(cJSON *object)
{
    if(object->type == cJSON_String)//字符串
        printf("%s:%s\n",object->string,object->valuestring);
    else if(object->type == cJSON_Number)//数字
        printf("%s:%d\n",object->string,object->valueint);
    else if(object->type == cJSON_Array)//数组
    {
        printf("%s:",object->string);
        //获取数组中元素的个数
        int num = cJSON_GetArraySize(object);
        for(int i = 0;i<num;i++)
        {
            cJSON *temp = cJSON_GetArrayItem(object,i);//获取数组元素
            if(temp->type == cJSON_String)
                printf("%s ",temp->valuestring);
            else if(temp->type == cJSON_Number)
                printf("%d ",temp->valueint);
        }
        printf("\n");
    }     
}
int main()
{
    char buf[] = "{\"name\":\"lixiang\",\"age\":18,\"score\":[80,90,100]}";
    //把字符串转成cjson格式
    cJSON *cjson_obj = cJSON_Parse(buf);

    //获取对象object中键值为string的json对象
    cJSON *cjosn_name = cJSON_GetObjectItem(cjson_obj,"name");    
    print(cjosn_name);

    cJSON *cjosn_age = cJSON_GetObjectItem(cjson_obj,"age");    
    print(cjosn_age);

    cJSON *cjosn_score = cJSON_GetObjectItem(cjson_obj,"score");    
    print(cjosn_score);
}
```

练习：

格式：

```
{"name":"luffy","sex":"man","age":19}
```

格式：

```
{"list":{"name":"luffy","age":19},"other":{"name":"ace"}}
```

格式：

```
{"names":["luffy","robin"]}
```

