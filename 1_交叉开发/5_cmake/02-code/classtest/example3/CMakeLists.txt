#cmake 最低版本号的要求
cmake_minimum_required(VERSION 3.10)

#项目名称 可以是任意的名字
project(demo1)

#查找当前目录下所有的源文件，并且保存到DIR_SRCS变量中
aux_source_directory(. DIR_SRCS)

#指定生成目标
# ${ xx } 表示取变量xx的值
add_executable(main ${DIR_SRCS})