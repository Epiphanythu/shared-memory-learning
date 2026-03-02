#!/bin/bash

# 共享内存Demo测试脚本
# 演示C++写入整数，Python读取的完整流程

echo "=== 共享内存Demo测试脚本 ==="
echo

# 检查是否在正确的目录
if [ ! -f "writer.cpp" ] || [ ! -f "reader.py" ]; then
    echo "错误：请在demo目录下运行此脚本"
    exit 1
fi

# 清理可能存在的旧共享内存对象
echo "1. 清理可能存在的旧共享内存对象..."
rm -f /dev/shm/demo_shm 2>/dev/null
echo "   ✓ 清理完成"

# 编译C++程序
echo
echo "2. 编译C++写入程序..."
if make clean && make; then
    echo "   ✓ 编译成功"
else
    echo "   ✗ 编译失败"
    exit 1
fi

# 检查Python环境
echo
echo "3. 检查Python环境..."
if command -v python3 &> /dev/null; then
    echo "   ✓ Python3 可用: $(python3 --version)"
else
    echo "   ✗ Python3 不可用，请安装Python3"
    exit 1
fi

# 检查必要的Python模块
echo "4. 检查Python模块..."
python3 -c "import mmap, os, struct, sys" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "   ✓ 所需Python模块可用"
else
    echo "   ✗ 缺少必要的Python模块"
    exit 1
fi

# 启动C++写入程序（后台运行）
echo
echo "5. 启动C++写入程序..."
echo "   注意：程序将在后台运行并等待Python读取完成"
echo

# 修改后的C++程序：在后台运行，但不立即清理
./writer > writer.log 2>&1 &
WRITER_PID=$!

# 等待C++程序创建共享内存
sleep 3

# 检查共享内存是否创建成功
if [ ! -e "/dev/shm/demo_shm" ]; then
    echo "   ✗ 共享内存创建失败"
    kill $WRITER_PID 2>/dev/null
    exit 1
fi

echo "   ✓ 共享内存创建成功"

# 运行Python读取程序
echo "6. 运行Python读取程序..."
python3 reader.py
READER_RESULT=$?

# 发送回车键给C++程序，让它清理资源
echo "7. 通知C++程序清理资源..."
echo -ne "\r" > /proc/$WRITER_PID/fd/0 2>/dev/null || {
    # 如果上面的方法失败，尝试其他方法
    echo "   发送信号终止C++程序..."
    kill -INT $WRITER_PID 2>/dev/null
}

# 等待C++程序完成
wait $WRITER_PID 2>/dev/null
WRITER_RESULT=$?

# 清理日志文件
rm -f writer.log

# 检查结果
echo
echo "=== 测试结果 ==="
if [ $READER_RESULT -eq 0 ] && [ $WRITER_RESULT -eq 0 ]; then
    echo "✓ 测试成功！C++成功写入整数，Python成功读取"
    echo
    echo "共享内存通信已完成！"
else
    echo "✗ 测试失败"
    echo "C++程序退出码: $WRITER_RESULT"
    echo "Python程序退出码: $READER_RESULT"
fi

echo
echo "=== 手动测试方法 ==="
echo "如果你想手动测试，请按以下步骤操作："
echo "1. 终端1: ./writer"
echo "2. 等待C++程序提示'现在可以运行: python3 reader.py'"
echo "3. 终端2: python3 reader.py"
echo "4. 在C++程序终端按回车键清理资源"
echo
echo "清理命令: ./clean.sh"

exit 0
