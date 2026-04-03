cmake_minimum_required(VERSION 3.10)

project(classtest C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

aux_source_directory(src SRC_LIST)

add_executable(classtest ${SRC_LIST})

target_include_directories(classtest PRIVATE inc)

