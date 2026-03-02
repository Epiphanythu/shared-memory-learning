#include <iostream>
#include <fcntl.h>           // 用于O_*常量
#include <sys/mman.h>        // 用于shm_open, mmap
#include <sys/stat.h>        // 用于权限常量
#include <unistd.h>          // 用于ftruncate, close
#include <cerrno>            // 用于errno
#include <cstring>           // 用于strerror

// 共享内存对象的名称（必须以/开头）
const char* SHM_NAME = "/demo_shm";
// 共享内存大小（一个整数，4字节）
const size_t SHM_SIZE = sizeof(int);

int main() {
    std::cout << "=== C++ 共享内存写入程序 ===" << std::endl;
    
    // 步骤1: 创建或打开共享内存对象
    // shm_open() 创建一个新的共享内存对象或打开一个已存在的
    // 参数: 名称, 标志, 权限
    // O_CREAT: 如果不存在则创建
    // O_RDWR: 以读写方式打开
    // O_EXCL: 如果已存在则失败（确保我们是创建者）
    std::cout << "1. 创建共享内存对象..." << std::endl;
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_EXCL, 0666);
    if (shm_fd == -1) {
        std::cerr << "shm_open 失败: " << strerror(errno) << std::endl;
        std::cerr << "可能原因: 共享内存对象已存在，请先运行 clean.sh 清理" << std::endl;
        return 1;
    }
    std::cout << "   ✓ 成功创建共享内存对象: " << SHM_NAME << std::endl;
    
    // 步骤2: 设置共享内存大小
    // ftruncate() 调整文件或共享内存的大小
    // 参数: 文件描述符, 新大小
    std::cout << "2. 设置共享内存大小..." << std::endl;
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        std::cerr << "ftruncate 失败: " << strerror(errno) << std::endl;
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }
    std::cout << "   ✓ 成功设置大小为: " << SHM_SIZE << " 字节" << std::endl;
    
    // 步骤3: 将共享内存映射到进程地址空间
    // mmap() 将文件或共享内存映射到内存
    // 参数: 地址(建议为NULL让系统选择), 大小, 保护标志, 映射标志, 文件描述符, 偏移量
    // PROT_READ | PROT_WRITE: 可读可写
    // MAP_SHARED: 对其他进程可见，修改会同步到共享内存
    std::cout << "3. 映射共享内存到地址空间..." << std::endl;
    void* shm_ptr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        std::cerr << "mmap 失败: " << strerror(errno) << std::endl;
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }
    std::cout << "   ✓ 成功映射到地址: " << shm_ptr << std::endl;
    
    // 步骤4: 写入数据到共享内存
    // 现在可以像操作普通内存一样操作共享内存
    int value_to_write = 42;  // 要写入的整数
    std::cout << "4. 写入数据到共享内存..." << std::endl;
    
    // 将void*转换为int*，然后写入数据
    int* int_ptr = static_cast<int*>(shm_ptr);
    *int_ptr = value_to_write;
    
    std::cout << "   ✓ 成功写入整数: " << value_to_write << std::endl;
    
    // 步骤5: 同步内存（确保数据写入到共享内存）
    // msync() 将内存中的更改同步到映射的文件或共享内存
    // MS_SYNC: 同步写入，等待完成
    std::cout << "5. 同步内存..." << std::endl;
    if (msync(shm_ptr, SHM_SIZE, MS_SYNC) == -1) {
        std::cerr << "msync 失败: " << strerror(errno) << std::endl;
        // 继续执行，因为可能不是致命错误
    } else {
        std::cout << "   ✓ 内存同步完成" << std::endl;
    }
    
    // 步骤6: 通知用户可以运行Python读取程序
    std::cout << "\n=== 写入完成 ===" << std::endl;
    std::cout << "共享内存已准备就绪！" << std::endl;
    std::cout << "现在可以运行: python3 reader.py" << std::endl;
    std::cout << "按回车键继续清理资源..." << std::endl;
    
    // 检查是否有终端输入（交互式运行）
    if (isatty(STDIN_FILENO)) {
        std::cin.get();  // 交互式运行时等待用户输入
    } else {
        // 后台运行时，等待共享内存被读取或超时
        std::cout << "检测到后台运行，等待Python程序读取..." << std::endl;
        
        // 等待最多30秒，或者等待共享内存被读取
        for (int i = 0; i < 30; ++i) {
            sleep(1);
            // 检查共享内存是否还存在（如果被Python读取后可能被删除）
            // 这里简单等待，实际应用中可以使用更复杂的同步机制
        }
        std::cout << "等待超时或检测到读取完成" << std::endl;
    }
    
    // 步骤7: 清理资源
    std::cout << "\n=== 清理资源 ===" << std::endl;
    
    // 解除内存映射
    std::cout << "7. 解除内存映射..." << std::endl;
    if (munmap(shm_ptr, SHM_SIZE) == -1) {
        std::cerr << "munmap 失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "   ✓ 内存映射已解除" << std::endl;
    }
    
    // 关闭文件描述符
    std::cout << "8. 关闭文件描述符..." << std::endl;
    if (close(shm_fd) == -1) {
        std::cerr << "close 失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "   ✓ 文件描述符已关闭" << std::endl;
    }
    
    // 删除共享内存对象
    std::cout << "9. 删除共享内存对象..." << std::endl;
    if (shm_unlink(SHM_NAME) == -1) {
        std::cerr << "shm_unlink 失败: " << strerror(errno) << std::endl;
    } else {
        std::cout << "   ✓ 共享内存对象已删除: " << SHM_NAME << std::endl;
    }
    
    std::cout << "\n程序执行完成！" << std::endl;
    return 0;
}
