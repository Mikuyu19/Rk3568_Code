add_library(add SHARED add.c)
add_library(sub STATIC sub.c)

target_include_directories(add PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
target_include_directories(sub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../inc)
