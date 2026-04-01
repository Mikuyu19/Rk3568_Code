CC = gcc
CFLAGS = -g -O0 
TARGET = app

SRCS = main.c add.c 
OBJS = $(SRCS:.c=.o)

$(TARGET):$(OBJS)
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY:clean
