# 生产者-消费者项目编译脚本
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
LDFLAGS = 

# 目标文件
PRODUCER = producer
CONSUMER = consumer
MONITOR = monitor

# 源文件
PRODUCER_SRC = producer.c
CONSUMER_SRC = consumer.c
MONITOR_SRC = monitor.c
HEADER_SRC = producer_consumer.h

.PHONY: all clean run-producer run-consumer run-monitor test demo clean-ipc help

# 编译所有程序
all: $(PRODUCER) $(CONSUMER) $(MONITOR)

# 编译生产者程序
$(PRODUCER): $(PRODUCER_SRC) $(HEADER_SRC)
	$(CC) $(CFLAGS) -o $(PRODUCER) $(PRODUCER_SRC) $(LDFLAGS)

# 编译消费者程序
$(CONSUMER): $(CONSUMER_SRC) $(HEADER_SRC)
	$(CC) $(CFLAGS) -o $(CONSUMER) $(CONSUMER_SRC) $(LDFLAGS)

# 编译监控程序
$(MONITOR): $(MONITOR_SRC) $(HEADER_SRC)
	$(CC) $(CFLAGS) -o $(MONITOR) $(MONITOR_SRC) $(LDFLAGS)

# 运行生产者（单个）
run-producer: $(PRODUCER)
	@echo "启动单个生产者..."
	./$(PRODUCER) 1 20 1000000

# 运行消费者（单个）
run-consumer: $(CONSUMER)
	@echo "启动单个消费者..."
	./$(CONSUMER) 1 20 1500000

# 运行监控程序
run-monitor: $(MONITOR)
	@echo "启动监控程序..."
	./$(MONITOR)

# 快速测试 - 一个生产者一个消费者
test: all
	@echo "=== 快速测试：1生产者 + 1消费者 ==="
	@echo "将在三个终端中分别启动："
	@echo "1. 生产者进程"
	@echo "2. 消费者进程" 
	@echo "3. 监控进程"
	@echo ""
	@echo "或使用 'make demo' 自动运行（需要xterm）"
	@echo ""

# 演示模式 - 自动启动多个终端
demo: all
	@if command -v xterm >/dev/null 2>&1; then \
		echo "启动演示模式..."; \
		xterm -title "Producer 1" -e "./$(PRODUCER) 1 15 1000000; read" & \
		xterm -title "Producer 2" -e "./$(PRODUCER) 2 15 800000; read" & \
		sleep 1 && \
		xterm -title "Consumer 1" -e "./$(CONSUMER) 1 20 1200000; read" & \
		xterm -title "Consumer 2" -e "./$(CONSUMER) 2 20 1800000; read" & \
		sleep 1 && \
		xterm -title "Monitor" -e "./$(MONITOR) -i 1; read"; \
	else \
		echo "需要安装xterm来运行演示"; \
		echo "请手动在多个终端中运行："; \
		echo "终端1: make run-producer"; \
		echo "终端2: make run-consumer"; \
		echo "终端3: make run-monitor"; \
	fi

# 压力测试 - 多生产者多消费者
stress-test: all
	@if command -v xterm >/dev/null 2>&1; then \
		echo "启动压力测试..."; \
		for i in 1 2 3; do \
			xterm -title "Producer $$i" -e "./$(PRODUCER) $$i 30 500000; read" & \
			xterm -title "Consumer $$i" -e "./$(CONSUMER) $$i 30 600000; read" & \
		done; \
		sleep 2 && \
		xterm -title "Stress Monitor" -e "./$(MONITOR) -i 1; read"; \
	else \
		echo "需要安装xterm来运行压力测试"; \
	fi

# 性能测试
perf-test: all
	@echo "=== 性能测试 ==="
	@echo "启动1个快速生产者和1个慢速消费者..."
	@echo "观察缓冲区积压情况"
	@if command -v xterm >/dev/null 2>&1; then \
		xterm -title "Fast Producer" -e "./$(PRODUCER) 1 50 200000; read" & \
		xterm -title "Slow Consumer" -e "./$(CONSUMER) 1 50 1000000; read" & \
		sleep 1 && \
		xterm -title "Performance Monitor" -e "./$(MONITOR) -i 1; read"; \
	else \
		echo "需要安装xterm来运行性能测试"; \
	fi

# 死锁测试（小心使用）
deadlock-test: all
	@echo "=== 死锁风险测试 ==="
	@echo "启动多个快速生产者，可能导致缓冲区满"
	@echo "按 Ctrl+C 停止测试"
	@if command -v xterm >/dev/null 2>&1; then \
		for i in 1 2 3 4 5; do \
			xterm -title "Fast Producer $$i" -e "./$(PRODUCER) $$i 100 100000; read" & \
		done; \
		sleep 2 && \
		xterm -title "Deadlock Monitor" -e "./$(MONITOR) -i 1; read"; \
	else \
		echo "需要安装xterm来运行死锁测试"; \
	fi

# 显示IPC资源状态
show-ipc:
	@echo "=== 当前IPC资源状态 ==="
	@echo "共享内存段:"
	@ipcs -m 2>/dev/null || echo "  无共享内存段"
	@echo ""
	@echo "信号量集:"
	@ipcs -s 2>/dev/null || echo "  无信号量集"
	@echo ""
	@echo "消息队列:"
	@ipcs -q 2>/dev/null || echo "  无消息队列"

# 清理IPC资源
clean-ipc:
	@echo "清理IPC资源..."
	@# 清理共享内存
	@for id in $$(ipcs -m 2>/dev/null | awk '/0x/ {print $$2}'); do \
		echo "删除共享内存段 $$id"; \
		ipcrm -m $$id 2>/dev/null || true; \
	done
	@# 清理信号量
	@for id in $$(ipcs -s 2>/dev/null | awk '/0x/ {print $$2}'); do \
		echo "删除信号量集 $$id"; \
		ipcrm -s $$id 2>/dev/null || true; \
	done
	@# 清理消息队列
	@for id in $$(ipcs -q 2>/dev/null | awk '/0x/ {print $$2}'); do \
		echo "删除消息队列 $$id"; \
		ipcrm -q $$id 2>/dev/null || true; \
	done
	@echo "IPC资源清理完成"

# 清理编译文件
clean:
	rm -f $(PRODUCER) $(CONSUMER) $(MONITOR)
	@echo "编译文件清理完成"

# 完全清理
clean-all: clean clean-ipc
	@echo "完全清理完成"

# 代码格式化
format:
	@if command -v clang-format >/dev/null 2>&1; then \
		echo "格式化代码..."; \
		clang-format -i *.c *.h; \
	else \
		echo "需要安装clang-format"; \
	fi

# 代码检查
check:
	@if command -v cppcheck >/dev/null 2>&1; then \
		echo "代码检查..."; \
		cppcheck --enable=all --std=c99 *.c *.h; \
	else \
		echo "需要安装cppcheck"; \
	fi

# 生成文档
docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "生成文档..."; \
		doxygen Doxyfile 2>/dev/null || echo "Doxyfile不存在"; \
	else \
		echo "需要安装doxygen"; \
	fi

# 安装程序到系统目录
install: all
	@echo "安装到 /usr/local/bin/..."
	@if [ $$EUID -eq 0 ]; then \
		cp $(PRODUCER) /usr/local/bin/; \
		cp $(CONSUMER) /usr/local/bin/; \
		cp $(MONITOR) /usr/local/bin/; \
		echo "安装完成"; \
	else \
		echo "需要root权限安装，使用 sudo make install"; \
	fi

# 卸载程序
uninstall:
	@echo "从 /usr/local/bin/ 卸载..."
	@if [ $$EUID -eq 0 ]; then \
		rm -f /usr/local/bin/$(PRODUCER); \
		rm -f /usr/local/bin/$(CONSUMER); \
		rm -f /usr/local/bin/$(MONITOR); \
		echo "卸载完成"; \
	else \
		echo "需要root权限卸载，使用 sudo make uninstall"; \
	fi

# 显示帮助
help:
	@echo "生产者-消费者共享内存项目"
	@echo "============================="
	@echo ""
	@echo "基本命令："
	@echo "  make all         - 编译所有程序"
	@echo "  make clean       - 清理编译文件"
	@echo "  make clean-ipc   - 清理IPC资源"
	@echo "  make clean-all   - 完全清理"
	@echo ""
	@echo "运行命令："
	@echo "  make run-producer - 运行单个生产者"
	@echo "  make run-consumer - 运行单个消费者"
	@echo "  make run-monitor  - 运行监控程序"
	@echo ""
	@echo "测试命令："
	@echo "  make test        - 快速测试说明"
	@echo "  make demo        - 演示模式（多终端）"
	@echo "  make stress-test - 压力测试"
	@echo "  make perf-test   - 性能测试"
	@echo "  make deadlock-test - 死锁风险测试"
	@echo ""
	@echo "工具命令："
	@echo "  make show-ipc    - 显示IPC资源状态"
	@echo "  make format      - 格式化代码"
	@echo "  make check       - 代码检查"
	@echo "  make docs        - 生成文档"
	@echo "  make install     - 安装到系统"
	@echo "  make uninstall   - 从系统卸载"
	@echo ""
	@echo "示例用法："
	@echo "  # 编译并运行基本测试"
	@echo "  make all && make demo"
	@echo ""
	@echo "  # 手动测试（三个终端）"
	@echo "  # 终端1: make run-producer"
	@echo "  # 终端2: make run-consumer" 
	@echo "  # 终端3: make run-monitor"
	@echo ""
	@echo "  # 自定义参数运行"
	@echo "  ./producer 1 50 500000    # 生产者1，生产50个产品，延迟0.5秒"
	@echo "  ./consumer 1 50 800000    # 消费者1，消费50个产品，延迟0.8秒"
	@echo "  ./monitor -i 3              # 监控程序，3秒更新间隔"
