#cmake 最低版本号的要求
cmake_minimum_required(VERSION 3.10)

#项目名称 可以是任意的名字
project(demo1)

set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
set(CMAKE_CXX_COMPILER "aarch64-linux-gnu-g++")

#指定生成目标
add_executable(main main.c)