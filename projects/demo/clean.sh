#!/bin/bash

# 共享内存Demo清理脚本
# 清理所有相关的资源

echo "=== 共享内存Demo清理脚本 ==="
echo

# 函数：检查命令是否成功执行
check_result() {
    if [ $? -eq 0 ]; then
        echo "   ✓ $1"
    else
        echo "   ✗ $1"
    fi
}

# 1. 清理编译生成的可执行文件
echo "1. 清理编译文件..."
if [ -f "writer" ]; then
    rm -f writer
    check_result "删除可执行文件: writer"
else
    echo "   - 可执行文件: writer 不存在"
fi

# 2. 清理共享内存对象
echo
echo "2. 清理共享内存对象..."
SHM_NAME="/demo_shm"

# 检查共享内存是否存在
if [ -e "/dev/shm$SHM_NAME" ]; then
    # 使用rm命令删除共享内存对象
    rm -f "/dev/shm$SHM_NAME"
    check_result "删除共享内存对象: $SHM_NAME"
else
    echo "   - 共享内存对象: $SHM_NAME 不存在"
fi

# 3. 使用ipcrm命令清理（备用方法）
echo
echo "3. 使用系统命令清理..."
# 查找并删除所有相关的共享内存段
SHM_IDS=$(ipcs -m | grep "0x" | grep -E "(demo|4321)" | awk '{print $2}')

if [ -n "$SHM_IDS" ]; then
    for shm_id in $SHM_IDS; do
        ipcrm -m $shm_id 2>/dev/null
        check_result "删除共享内存段ID: $shm_id"
    done
else
    echo "   - 没有找到相关的共享内存段"
fi

# 4. 清理临时文件
echo
echo "4. 清理临时文件..."
if [ -f "auto_input.exp" ]; then
    rm -f auto_input.exp
    check_result "删除临时脚本: auto_input.exp"
else
    echo "   - 临时脚本: auto_input.exp 不存在"
fi

# 5. 使用make clean清理
echo
echo "5. 使用Make清理..."
if [ -f "Makefile" ]; then
    make clean >/dev/null 2>&1
    check_result "执行 make clean"
else
    echo "   - Makefile 不存在"
fi

# 6. 显示当前共享内存状态
echo
echo "6. 检查清理结果..."
echo "当前共享内存状态："
ipcs -m | head -1  # 显示标题行
ipcs -m | grep -E "(demo|4321)" || echo "   (没有找到相关的共享内存段)"

# 7. 清理日志文件（如果存在）
echo
echo "7. 清理日志文件..."
rm -f *.log 2>/dev/null
echo "   ✓ 日志文件清理完成"

echo
echo "=== 清理完成 ==="
echo "所有与共享内存Demo相关的资源已清理"
echo
echo "如果仍有问题，可以手动执行："
echo "  sudo ipcs -m  # 查看所有共享内存"
echo "  sudo ipcrm -m <ID>  # 删除指定ID的共享内存"
