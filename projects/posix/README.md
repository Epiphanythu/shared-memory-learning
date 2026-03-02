# POSIX共享内存示例

本目录包含了POSIX共享内存的完整示例，展示如何使用共享内存和信号量实现进程间通信。

## 文件说明

- `simple_writer.c` - 写入进程，向共享内存写入数据
- `simple_reader.c` - 读取进程，从共享内存读取数据
- `Makefile` - 编译和运行脚本
- `README.md` - 本说明文件

## 核心API说明

### 1. 共享内存操作

#### `shm_open()`
```c
int shm_open(const char *name, int oflag, mode_t mode);
```
- **功能**: 创建或打开共享内存对象
- **参数**:
  - `name`: 共享内存名称（必须以"/"开头）
  - `oflag`: 打开标志（O_CREAT, O_RDWR, O_RDONLY等）
  - `mode`: 权限标志
- **返回值**: 文件描述符，失败返回-1

#### `ftruncate()`
```c
int ftruncate(int fd, off_t length);
```
- **功能**: 设置共享内存大小
- **参数**:
  - `fd`: 共享内存文件描述符
  - `length`: 要设置的大小
- **返回值**: 成功返回0，失败返回-1

#### `mmap()`
```c
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```
- **功能**: 将共享内存映射到进程地址空间
- **参数**:
  - `addr`: 建议的映射地址（通常为NULL）
  - `length`: 映射大小
  - `prot`: 保护标志（PROT_READ, PROT_WRITE等）
  - `flags`: 映射标志（MAP_SHARED, MAP_PRIVATE等）
  - `fd`: 文件描述符
  - `offset`: 偏移量
- **返回值**: 映射地址，失败返回MAP_FAILED

### 2. 信号量操作

#### `sem_open()`
```c
sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
```
- **功能**: 创建或打开命名信号量
- **参数**:
  - `name`: 信号量名称（必须以"/"开头）
  - `oflag`: 打开标志（O_CREAT, O_EXCL等）
  - `mode`: 权限标志
  - `value`: 信号量初始值
- **返回值**: 信号量指针，失败返回SEM_FAILED

#### `sem_wait()` / `sem_post()`
```c
int sem_wait(sem_t *sem);  // 等待（P操作）
int sem_post(sem_t *sem);  // 释放（V操作）
```
- **功能**: 获取和释放信号量
- **返回值**: 成功返回0，失败返回-1

## 编译和运行

### 1. 编译程序
```bash
make
```

### 2. 运行测试
需要打开两个终端：

**终端1（写入进程）**:
```bash
make run-writer
```

**终端2（读取进程）**:
```bash
make run-reader
```

### 3. 自动测试（如果有xterm）
```bash
make test-xterm
```

### 4. 清理资源
```bash
make clean
```

## 程序流程

### 写入程序流程
1. 创建共享内存对象
2. 设置共享内存大小
3. 将共享内存映射到地址空间
4. 创建信号量
5. 循环写入数据（每秒一条）
6. 等待用户输入后清理资源

### 读取程序流程
1. 打开已存在的共享内存对象
2. 将共享内存映射到地址空间
3. 打开信号量
4. 循环检查并读取新数据
5. 读取完成后清理资源

## 数据结构

```c
typedef struct {
    time_t timestamp;    // 时间戳
    pid_t writer_pid;    // 写入进程PID
    char message[256];   // 消息内容
    int counter;         // 计数器
} shared_data_t;
```

## 同步机制

本示例使用命名信号量来实现进程间的同步：

- **互斥访问**: 使用二进制信号量确保对共享数据的互斥访问
- **数据一致性**: 在写入和读取时都获取信号量，避免竞态条件
- **资源管理**: 程序结束时正确关闭和清理信号量

## 常见问题和解决方案

### 1. 权限问题
如果遇到权限错误，确保：
- 程序有足够的权限创建共享内存对象
- 检查`/dev/shm`目录的权限

### 2. 资源清理
如果程序异常退出，可能导致共享内存对象未清理：
```bash
# 查看共享内存对象
ls -la /dev/shm/

# 手动清理
rm -f /dev/shm/my_shared_memory
rm -f /dev/shm/sem.my_semaphore
```

### 3. 调试技巧
- 使用`ipcs -m`查看系统共享内存段
- 使用`strace`跟踪系统调用
- 添加调试输出查看程序执行流程

## 扩展练习

1. **多读者单写者**: 实现多个读取进程同时读取数据
2. **环形缓冲区**: 实现一个环形缓冲区的共享内存
3. **无锁编程**: 使用原子操作替代信号量
4. **性能测试**: 测试不同数据大小的传输性能
5. **错误处理**: 增强错误处理和恢复机制

## 相关资源

- `man shm_open` - 共享内存打开函数
- `man mmap` - 内存映射函数
- `man sem_open` - 信号量打开函数
- `man ftruncate` - 文件截断函数
- POSIX标准文档
