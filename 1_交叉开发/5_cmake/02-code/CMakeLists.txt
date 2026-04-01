# 1. 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 2. 项目信息
project(RK3568_BST_Demo)

# 3. 设置头文件搜索路径 (对应 gcc 的 -I ./)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 4. 编译动态库 (对应 gcc -shared -fpic -o libbst.so bst.c linkqueue.c)
# 格式：add_library(库名 类型 源码文件)
# 注意：库名写 bst 即可，CMake 会自动在 Linux 下生成 libbst.so
add_library(bst SHARED bst.c linkqueue.c)

# 5. 编译可执行程序 (对应 gcc main.c -o app)
add_executable(app main.c)

# 6. 链接动态库 (对应 gcc ... -L./ -lbst)
# 告诉 CMake，app 运行需要用到 bst 这个库
target_link_libraries(app bst)
