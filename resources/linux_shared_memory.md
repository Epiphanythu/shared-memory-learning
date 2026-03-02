# Linux/Unix共享内存机制

## 概述

Linux/Unix系统提供了多种共享内存实现机制，主要包括：
1. System V共享内存
2. POSIX共享内存
3. 内存映射文件(mmap)

每种机制都有其特定的API和使用场景。

## System V共享内存

### 特点
- 传统的Unix IPC机制
- 使用键值(key)标识共享内存段
- 通过`ipcs`和`ipcrm`命令管理
- 在所有Unix系统上广泛支持

### 核心API

#### 1. 创建/获取共享内存段
```c
#include <sys/shm.h>
int shmget(key_t key, size_t size, int shmflg);
```
- `key`: 共享内存键值（可以是IPC_PRIVATE）
- `size`: 共享内存大小
- `shmflg`: 创建标志和权限
- 返回值：共享内存标识符

#### 2. 附加到进程地址空间
```c
void *shmat(int shmid, const void *shmaddr, int shmflg);
```
- `shmid`: 共享内存标识符
- `shmaddr`: 建议的附加地址（通常为NULL）
- `shmflg`: 附加标志
- 返回值：附加后的地址

#### 3. 分离共享内存
```c
int shmdt(const void *shmaddr);
```
- `shmaddr`: 要分离的地址
- 返回值：成功返回0，失败返回-1

#### 4. 控制操作
```c
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```
- `cmd`: 命令类型（IPC_STAT, IPC_SET, IPC_RMID等）
- `buf`: 缓冲区指针

### 管理命令
```bash
# 查看共享内存段
ipcs -m

# 删除共享内存段
ipcrm -m shmid

# 查看详细信息
ipcs -m -i shmid
```

## POSIX共享内存

### 特点
- 更现代的API设计
- 使用文件系统路径标识
- 与文件操作更相似
- 符合POSIX标准

### 核心API

#### 1. 创建/打开共享内存对象
```c
#include <sys/mman.h>
#include <fcntl.h>
int shm_open(const char *name, int oflag, mode_t mode);
```
- `name`: 共享内存名称（必须以"/"开头）
- `oflag`: 打开标志
- `mode`: 权限模式
- 返回值：文件描述符

#### 2. 设置大小
```c
int ftruncate(int fd, off_t length);
```

#### 3. 内存映射
```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

#### 4. 取消映射
```c
int munmap(void *addr, size_t length);
```

#### 5. 关闭和删除
```c
int close(int fd);
int shm_unlink(const char *name);
```

### 文件系统位置
POSIX共享内存对象通常挂载在`/dev/shm/`目录下：
```bash
# 查看共享内存对象
ls -la /dev/shm/

# 手动清理
rm -f /dev/shm/my_shared_memory
```

## 内存映射文件(mmap)

### 特点
- 将文件映射到内存空间
- 既可用于IPC，也可用于文件I/O
- 支持匿名映射（不依赖文件）
- 灵活的内存管理

### 核心API

#### 1. 内存映射
```c
#include <sys/mman.h>
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

#### 2. 同步内存到文件
```c
int msync(void *addr, size_t length, int flags);
```

#### 3. 设置内存保护
```c
int mprotect(void *addr, size_t len, int prot);
```

#### 4. 内存建议
```c
int madvise(void *addr, size_t length, int advice);
```

### 映射类型
- `MAP_SHARED`: 共享映射，修改对其他进程可见
- `MAP_PRIVATE`: 私有映射，修改只对当前进程可见
- `MAP_ANONYMOUS`: 匿名映射，不依赖文件

## 系统配置

### 内核参数
```bash
# 查看共享内存限制
cat /proc/sys/kernel/shmmax    # 最大共享内存段大小
cat /proc/sys/kernel/shmmni    # 最大共享内存段数量
cat /proc/sys/kernel/shmall    # 系统总共享内存页数

# 临时修改
echo 134217728 > /proc/sys/kernel/shmmax

# 永久修改（/etc/sysctl.conf）
kernel.shmmax = 134217728
kernel.shmmni = 4096
kernel.shmall = 2097152
```

### 用户限制
```bash
# 查看用户限制
ulimit -a

# 设置共享内存限制
ulimit -m unlimited  # 内存限制
```

## 性能优化

### 1. 页面大小对齐
```c
long page_size = sysconf(_SC_PAGESIZE);
size_t aligned_size = ((size + page_size - 1) / page_size) * page_size;
```

### 2. 大页支持
```bash
# 检查大页支持
cat /proc/meminfo | grep Huge

# 配置大页
echo 1024 > /proc/sys/vm/nr_hugepages
```

### 3. NUMA优化
```bash
# 查看NUMA拓扑
numactl --hardware

# 在指定NUMA节点分配内存
numactl --cpunodebind=0 --membind=0 ./program
```

## 调试和监控

### 1. 系统工具
```bash
# 查看进程内存映射
cat /proc/pid/maps
pmap pid

# 查看共享内存使用
ipcs -m -u

# 监控内存使用
vmstat
free -h
```

### 2. 调试工具
```bash
# 使用strace跟踪系统调用
strace -e trace=shmget,shmat,shmdt ./program

# 使用ltrace跟踪库调用
ltrace ./program

# 使用GDB调试
gdb ./program
(gdb) print shmget(1234, 1024, IPC_CREAT|0666)
```

## 安全考虑

### 1. 权限控制
- 正确设置共享内存权限
- 限制访问用户组
- 使用适当的umask

### 2. 数据保护
- 避免敏感数据存储在共享内存
- 使用加密保护重要数据
- 定期清理共享内存

### 3. 资源限制
- 设置合理的共享内存大小限制
- 监控共享内存使用情况
- 防止资源耗尽攻击

## 常见问题

### 1. 权限拒绝
```c
// 解决方案：检查权限设置
shmget(key, size, IPC_CREAT | 0666);
```

### 2. 内存不足
```bash
# 检查系统内存
free -h
cat /proc/meminfo

# 调整内核参数
echo 268435456 > /proc/sys/kernel/shmmax
```

### 3. 资源泄漏
```bash
# 清理僵尸共享内存段
ipcs -m | grep "0x00000000" | awk '{print $2}' | xargs -r ipcrm -m
```

## 最佳实践

1. **错误处理**：始终检查系统调用的返回值
2. **资源清理**：确保在程序退出时清理资源
3. **同步机制**：使用适当的同步原语
4. **性能考虑**：考虑缓存对齐和NUMA影响
5. **安全第一**：最小权限原则和数据保护

## 相关文档

- `man shmget` - System V共享内存创建
- `man shmat` - System V共享内存附加
- `man shmctl` - System V共享内存控制
- `man shm_open` - POSIX共享内存打开
- `man mmap` - 内存映射
- `man ipcs` - IPC资源查看
- `man ipcrm` - IPC资源删除
