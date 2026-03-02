# Makefile for Shared Memory Demo
# 用于编译C++共享内存写入程序

# 编译器设置
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

# 目标文件
TARGET = writer

# 源文件
SOURCE = writer.cpp

# 链接库（POSIX共享内存需要链接rt库）
LIBS = -lrt

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(SOURCE)
	@echo "正在编译 C++ 共享内存写入程序..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LIBS)
	@echo "编译完成！可执行文件: $(TARGET)"

# 清理目标
clean:
	@echo "清理编译文件..."
	rm -f $(TARGET)
	@echo "清理完成！"

# 测试目标（编译并运行）
test: $(TARGET)
	@echo "开始测试..."
	@echo "注意：程序将等待你按回车键，此时可以在另一个终端运行 python3 reader.py"
	./$(TARGET)

# 帮助信息
help:
	@echo "可用的 make 目标："
	@echo "  all     - 编译程序（默认）"
	@echo "  clean   - 清理编译文件"
	@echo "  test    - 编译并运行程序"
	@echo "  help    - 显示此帮助信息"

# 声明伪目标
.PHONY: all clean test help
