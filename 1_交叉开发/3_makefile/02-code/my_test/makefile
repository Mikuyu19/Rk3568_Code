# # 1. 定义变量
# CC = gcc
# CFLAGS = -I./ -fpic
# LDFLAGS = -L./ -lbst

# # 2. 最终目标
# all: app

# # 3. 目标1：生成动态库
# # 依赖 bst.c 和 linkqueue.c
# libbst.so: bst.c linkqueue.c
# 	$(CC) -shared $(CFLAGS) -o $@ $^

# # 4. 目标2：生成可执行文件
# # 依赖 main.c 和 动态库
# app: main.c libbst.so
# 	$(CC) main.c $(CFLAGS) $(LDFLAGS) -o $@

# # 5. 清理规则
# clean:
# 	rm -rf libbst.so app
# .PHONY: all clean


# 自定义变量+自动变量+模式匹配+内置函数
# 不使用libbst.so，直接编译生成可执行文件
# 1. 定义变量
CC = gcc	

# 2. 定义目标和依赖
TARGET = app
SRCS = $(wildcard ./src/*.c)
OBJS = $(patsubst %.c, %.o, $(SRCS))

# 3. 最终目标
all: $(TARGET)

# 4. 生成可执行文件
$(TARGET): $(OBJS)
	$(CC) -o $@ $^	

# 5. 生成目标文件
%.o: %.c
	$(CC) -c $< -o $@

# 6. 清理规则
clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean



