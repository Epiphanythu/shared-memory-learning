#!/usr/bin/env python3
import os
import mmap
import sys
import struct

# 共享内存对象的名称（必须与C++程序中的名称一致）
SHM_NAME = "/demo_shm"
# 共享内存大小（一个整数，4字节）
SHM_SIZE = 4  # sizeof(int)

def main():
    print("=== Python 共享内存读取程序 ===")
    
    try:
        # 步骤1: 打开已存在的共享内存对象
        # 方法1: 尝试使用 os.open_shm() (Python 3.8+)
        print("1. 打开共享内存对象...")
        try:
            shm_fd = os.open_shm(SHM_NAME, os.O_RDONLY)
            print(f"   ✓ 成功打开共享内存对象: {SHM_NAME}")
        except AttributeError:
            # 方法2: 使用传统方法，通过文件系统路径打开
            print("   使用传统方法打开共享内存...")
            shm_path = f"/dev/shm{SHM_NAME}"
            shm_fd = os.open(shm_path, os.O_RDONLY)
            print(f"   ✓ 成功打开共享内存对象: {SHM_NAME}")
        
    except FileNotFoundError:
        print(f"   ✗ 错误: 找不到共享内存对象 '{SHM_NAME}'")
        print("   请先运行 C++ 写入程序: ./writer")
        return 1
    except OSError as e:
        print(f"   ✗ 打开共享内存失败: {e}")
        return 1
    
    try:
        # 步骤2: 将共享内存映射到进程地址空间
        # mmap.mmap() 创建内存映射对象
        # 参数: 文件描述符, 长度, 访问模式, 偏移量
        # mmap.ACCESS_READ: 只读访问
        print("2. 映射共享内存到地址空间...")
        
        # 创建内存映射对象
        shm_map = mmap.mmap(shm_fd, SHM_SIZE, access=mmap.ACCESS_READ)
        print(f"   ✓ 成功映射共享内存，大小: {SHM_SIZE} 字节")
        
    except Exception as e:
        print(f"   ✗ 内存映射失败: {e}")
        os.close(shm_fd)
        return 1
    
    try:
        # 步骤3: 读取共享内存中的数据
        print("3. 读取共享内存中的数据...")
        
        # 方法1: 直接读取字节并使用struct解包
        # struct.unpack() 将字节转换为指定格式的数据
        # 'i' 表示有符号整数（4字节）
        # '<' 表示小端字节序（与大多数系统兼容）
        data = shm_map.read(SHM_SIZE)
        if len(data) == SHM_SIZE:
            value = struct.unpack('<i', data)[0]
            print(f"   ✓ 读取到整数: {value}")
        else:
            print(f"   ✗ 读取的数据长度不正确: 期望{SHM_SIZE}字节，实际{len(data)}字节")
            return 1
            
        # 方法2: 备用方法 - 使用memoryview（更高效）
        # 注意：这里我们演示两种方法，但实际只使用一种
        print("4. 使用memoryview验证读取...")
        shm_map.seek(0)  # 重置指针到开始位置
        mv = memoryview(shm_map)
        value_verify = struct.unpack('<i', mv[0:4])[0]
        print(f"   ✓ 验证读取到整数: {value_verify}")
        
        if value == value_verify:
            print("   ✓ 两种方法读取结果一致")
        else:
            print(f"   ✗ 读取结果不一致: {value} vs {value_verify}")
            
    except Exception as e:
        print(f"   ✗ 读取数据失败: {e}")
        return 1
    
    finally:
        # 步骤4: 清理资源
        print("\n=== 清理资源 ===")
        
        # 关闭内存映射
        print("4. 关闭内存映射...")
        try:
            shm_map.close()
            print("   ✓ 内存映射已关闭")
        except Exception as e:
            print(f"   ✗ 关闭内存映射失败: {e}")
        
        # 关闭文件描述符
        print("5. 关闭文件描述符...")
        try:
            os.close(shm_fd)
            print("   ✓ 文件描述符已关闭")
        except Exception as e:
            print(f"   ✗ 关闭文件描述符失败: {e}")
    
    print("\n=== 读取成功 ===")
    print(f"成功从共享内存 '{SHM_NAME}' 读取到整数: {value}")
    
    # 额外信息
    print("\n=== 调试信息 ===")
    print(f"共享内存名称: {SHM_NAME}")
    print(f"共享内存大小: {SHM_SIZE} 字节")
    print(f"读取的整数: {value}")
    print(f"整数十六进制: 0x{value:08x}")
    print(f"整数二进制: {bin(value & 0xffffffff)}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
