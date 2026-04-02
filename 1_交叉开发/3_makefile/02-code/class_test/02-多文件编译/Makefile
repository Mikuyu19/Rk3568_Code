# main:main.o add.o sub.o
# 	gcc main.o add.o sub.o -o main

# main.o:main.c
# 	gcc -c main.c -o main.o
# add.o:add.c
# 	gcc -c add.c -o add.o
# sub.o:sub.c
# 	gcc -c sub.c -o sub.o

# clean:
# 	rm main.o add.o sub.o main

# 1. 定义变量
CC = gcc
TARGET = app
SRCS = main.c add.c sub.c
OBJS = $(SRCS:.c=.o)

# 2. 定义目标
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# 3. 定义编译规则
%.o: %.c
	$(CC) -c $< -o $@

# 4. 定义清理规则:
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: clean
