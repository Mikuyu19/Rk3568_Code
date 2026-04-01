# 1. 定义变量，方便以后修改（比如换成交叉编译器）
CC = gcc
CFLAGS = -I./ -fpic
LDFLAGS = -L./ -lbst

# 2. 最终目标
all: app

# 3. 目标1：生成动态库
# 依赖 bst.c 和 linkqueue.c
libbst.so: bst.c linkqueue.c
	$(CC) -shared $(CFLAGS) -o $@ $^

# 4. 目标2：生成可执行文件
# 依赖 main.c 和 动态库
app: main.c libbst.so
	$(CC) main.c $(CFLAGS) $(LDFLAGS) -o $@

# 5. 清理规则
clean:
	rm -f app libbst.so
