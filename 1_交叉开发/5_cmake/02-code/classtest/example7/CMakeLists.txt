#cmake 最低版本号的要求
cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/local/arm/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-g++)

#项目名称 可以是任意的名字
project(demo1)

#查找当前目录下src目录中所有的源文件，并且保存到DIR_SRCS变量中
aux_source_directory(./src DIR_SRCS)

#指定头文件路径
include_directories(./inc)

#指定库文件所在的路径
link_directories(./lib)

#添加lib子目录
add_subdirectory(./lib)

#指定生成目标
# ${ xx } 表示取变量xx的值
add_executable(main ${DIR_SRCS})

#指定链接的库的名字
target_link_libraries(main add sub)