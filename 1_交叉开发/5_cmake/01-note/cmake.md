# 1.cmake是什么？

cmake是一款跨平台的免费的开源软件工具，用于使用与编译器无关的方法来管理软件的构建的过程，LInux环境下有大量的项目的时候，都使用cmake来管理

# 2.安装cmake

1.在线安装

ubuntu有网的情况下下载cmake

```
sudo snap install cmake --classic
```

如果失败请更新源(请参考linux基本命令章节)

2.离线安装

(1)在cmake官网[Download CMake](https://cmake.org/download/)下载cmake

(2)将下载的压缩包放入到ubuntu的共享文件夹

(3)将压缩包解压到 家目录

```
tar zxvf cmake-4.0.1-linux-x86_64.tar.gz -C ~/
```

(4)配置环境变量

打开家目录下的.bashrc文件，在末尾添加如下语句

```
export PATH=$PATH:/home/china/cmake-4.0.1-linux-x86_64/bin
```

![1745976491910](cmake.assets/1745976491910.png)

重启配置文件

```c
source ~/.bashrc
```

检查：在shell终端输入cmake -version，得到版本信息，说明cmake安装完成

![1745976586023](cmake.assets/1745976586023.png)

# 3.cmake的构建

文件结构如下

```
example1
├── CMakeLists.txt
└── main.c
```

**执行 cmake命令**

注意：一般不会直接在当前的目录执行cmake,因为生成的文件跟源文件放在一起，若要删除要单独找出来再删除

一般做法

```
mkdir build	//在当前的目录下创建build目录
cd build //进入build目录
cmake ..	//编译上一层的CMakeLists.txt文件
```

**执行make命令**

```
make 		//编译Makefile文件，得到可执行程序main
./main		//运行可执行文件
```

## 设定cmake的最低版本号

> 语法：
>
> cmake_minimum_required(VERSION x.x)
>

## 设定项目名称

> 语法：
>
> project(PRO_NAME)
>

多文件编译

> 语法：
>
> ```
> add_executable(可执行文件名 源文件1 源文件2 ...)
> ```
>

## 查找源文件保存到变量中

> 语法：
>
> aux_source_directory(dir variable)
>
> dir:表示指定要查找的目录
>
> variable：表示变量名
>
> 功能：把dir目录中查找到的源文件的名字保存到变量variable

## 指定头文件路径

> 语法：
>
> include_directories(头文件所在的路径)

## 指定库路径和链接指定的库文件

> 语法：
>
> link_directories(库所在的路径)
>
> target_link_libraries(可执行文件名 库名1 库名2 ...)

## 添加工程子目录和编译库文件

> 语法：
>
> add_subdirectory(子目录名称)
>
> add_library(库名 [库的类型] 源文件1 源文件2)  //编译库
>
> 库类型：SHARED动态库 STATIC静态库
>
> 
>
> 公开头文件目录(提供给外部使用)
>
> target_include_directories(库名 PULIC 头文件所在的路径名)
>
> 

## 设置变量和调用shell命令

> 语法：
>
> set(变量名称 变量值）
>

## 指定工具链

> 语法：
>
> set(CMAKE_C_COMPILER "C编译器名字")
>
> set(CMAKE_CXX_COMPILER "C++编译器名字")

