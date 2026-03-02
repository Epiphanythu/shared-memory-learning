# POSIX共享内存示例编译脚本
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread
LDFLAGS = -lrt -pthread

# 目标文件
WRITER = simple_writer
READER = simple_reader

# 源文件
WRITER_SRC = simple_writer.c
READER_SRC = simple_reader.c

.PHONY: all clean run-writer run-reader test

all: $(WRITER) $(READER)

# 编译写入程序
$(WRITER): $(WRITER_SRC)
	$(CC) $(CFLAGS) -o $(WRITER) $(WRITER_SRC) $(LDFLAGS)

# 编译读取程序
$(READER): $(READER_SRC)
	$(CC) $(CFLAGS) -o $(READER) $(READER_SRC) $(LDFLAGS)

# 运行写入程序
run-writer: $(WRITER)
	./$(WRITER)

# 运行读取程序
run-reader: $(READER)
	./$(READER)

# 测试：先启动写入程序，然后在另一个终端启动读取程序
test: all
	@echo "测试说明："
	@echo "1. 在一个终端运行: make run-writer"
	@echo "2. 在另一个终端运行: make run-reader"
	@echo "3. 观察两个程序的输出"
	@echo ""
	@echo "或者使用以下命令同时运行（需要xterm）:"
	@echo "make test-xterm"

# 使用xterm同时运行两个程序（如果有xterm）
test-xterm: all
	@if command -v xterm >/dev/null 2>&1; then \
		xterm -e "make run-reader" & \
		sleep 1 && \
		xterm -e "make run-writer"; \
	else \
		echo "需要安装xterm来运行此测试"; \
		echo "请手动在两个终端中分别运行程序"; \
	fi

# 清理编译文件和共享资源
clean:
	rm -f $(WRITER) $(READER)
	@echo "清理可能的共享内存资源..."
	@-shmctl `ipcs -m | awk '/0x/ {print $2}'` 2>/dev/null || true
	@-rm -f /dev/shm/my_shared_memory 2>/dev/null || true
	@-rm -f /dev/shm/sem.my_semaphore 2>/dev/null || true

# 显示帮助
help:
	@echo "可用目标："
	@echo "  all        - 编译所有程序"
	@echo "  run-writer - 运行写入程序"
	@echo "  run-reader - 运行读取程序"
	@echo "  test       - 显示测试说明"
	@echo "  test-xterm - 使用xterm同时运行两个程序"
	@echo "  clean      - 清理编译文件和资源"
	@echo "  help       - 显示此帮助信息"
	@echo ""
	@echo "使用方法："
	@echo "1. 运行 'make' 编译程序"
	@echo "2. 在终端1运行: make run-writer"
	@echo "3. 在终端2运行: make run-reader"
	@echo "4. 观察程序输出"
