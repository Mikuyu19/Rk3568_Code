# unix域协议

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

