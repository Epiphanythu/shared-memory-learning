# System V共享内存示例编译脚本
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = 

# 目标文件
WRITER = sysv_writer
READER = sysv_reader

# 源文件
WRITER_SRC = sysv_writer.c
READER_SRC = sysv_reader.c

.PHONY: all clean run-writer run-reader test show-ipc clean-ipc

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
	@echo "辅助命令："
	@echo "  make show-ipc  - 显示当前的IPC资源"
	@echo "  make clean-ipc - 清理IPC资源"
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

# 显示当前IPC资源
show-ipc:
	@echo "=== 共享内存段 ==="
	@ipcs -m
	@echo ""
	@echo "=== 信号量集 ==="
	@ipcs -s
	@echo ""
	@echo "=== 消息队列 ==="
	@ipcs -q

# 清理IPC资源
clean-ipc:
	@echo "清理System V IPC资源..."
	@# 清理共享内存
	@for id in $$(ipcs -m | awk '/0x/ {print $$2}'); do \
		echo "删除共享内存段 $$id"; \
		ipcrm -m $$id 2>/dev/null || true; \
	done
	@# 清理信号量
	@for id in $$(ipcs -s | awk '/0x/ {print $$2}'); do \
		echo "删除信号量集 $$id"; \
		ipcrm -s $$id 2>/dev/null || true; \
	done
	@# 清理消息队列
	@for id in $$(ipcs -q | awk '/0x/ {print $$2}'); do \
		echo "删除消息队列 $$id"; \
		ipcrm -q $$id 2>/dev/null || true; \
	done
	@echo "IPC资源清理完成"

# 清理编译文件
clean:
	rm -f $(WRITER) $(READER)
	@echo "编译文件清理完成"

# 完全清理（编译文件 + IPC资源）
clean-all: clean clean-ipc

# 显示帮助
help:
	@echo "可用目标："
	@echo "  all         - 编译所有程序"
	@echo "  run-writer  - 运行写入程序"
	@echo "  run-reader  - 运行读取程序"
	@echo "  test        - 显示测试说明"
	@echo "  test-xterm  - 使用xterm同时运行两个程序"
	@echo "  show-ipc    - 显示当前IPC资源"
	@echo "  clean-ipc   - 清理IPC资源"
	@echo "  clean       - 清理编译文件"
	@echo "  clean-all   - 清理编译文件和IPC资源"
	@echo "  help        - 显示此帮助信息"
	@echo ""
	@echo "使用方法："
	@echo "1. 运行 'make' 编译程序"
	@echo "2. 在终端1运行: make run-writer"
	@echo "3. 在终端2运行: make run-reader"
	@echo "4. 观察程序输出"
	@echo "5. 运行 'make clean-all' 清理所有资源"
	@echo ""
	@echo "调试命令："
	@echo "- make show-ipc  查看系统IPC资源状态"
	@echo "- make clean-ipc 手动清理IPC资源"
