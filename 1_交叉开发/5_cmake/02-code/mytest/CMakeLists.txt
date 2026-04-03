# 1. 指定 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 2. 项目信息
project(app)

# 3. 设置头文件搜索路径 (对应 gcc 的 -I ./)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# 4. 编译可执行程序 (对应 gcc main.c -o app)
add_executable(app main.c bst.c linkqueue.c)

# 5. 设置编译选项 (对应 gcc 的 -g -Wall)
target_compile_options(app PRIVATE -g -Wall)

# 6. 设置链接选项 (对应 gcc 的 -lm)
target_link_libraries(app PRIVATE m)

# 7. 设置安装规则 (对应 make install)
install(TARGETS app DESTINATION bin)
