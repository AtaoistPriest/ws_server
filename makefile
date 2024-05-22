# 设置编译器
CC=gcc

# 设置编译选项
CFLAGS=-Wall -g -std=c99

# 设置源文件目录和目标文件目录
SRCDIR=./src

# 查找所有 .c 文件，并将路径存储到 SRC 变量
SRC=$(shell find $(SRCDIR) -name '*.c')

# 设置生成可执行文件的名称和路径
TARGET= ./bin/test

all:
	$(CC) $(CFLAGS) test.c $(SRC) -o $(TARGET) -lpthread -lssl -lcrypto -g

clean:
	rm -rf $(TARGET)
