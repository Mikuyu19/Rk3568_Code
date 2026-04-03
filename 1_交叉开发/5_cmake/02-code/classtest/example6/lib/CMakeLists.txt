#添加库 进行编译生成库
add_library(add SHARED add.c)
add_library(sub STATIC sub.c) #静态库是默认的 此处的 STATIC 可以不写

#公开头文件目录(供外部使用)
target_include_directories(add PUBLIC ../inc)
target_include_directories(sub PUBLIC ../inc)