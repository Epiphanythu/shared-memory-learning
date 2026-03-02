# Windows共享内存机制

## 概述

Windows操作系统提供了多种进程间通信机制，其中共享内存是最高效的一种。Windows的共享内存实现与Unix系统有所不同，主要通过以下几种方式实现：

1. 文件映射对象(File Mapping Objects)
2. 命名共享内存(Named Shared Memory)
3. 匿名共享内存(Anonymous Shared Memory)

## 核心概念

### 1. 文件映射对象
文件映射对象是Windows中实现共享内存的核心机制，它将文件或系统页面文件映射到进程的虚拟地址空间。

### 2. 页面文件
Windows使用页面文件(pagefile.sys)作为后备存储，支持内存映射操作。

### 3. 安全描述符
Windows共享内存支持细粒度的权限控制，通过安全描述符管理访问权限。

## 核心API

### 1. 创建文件映射对象
```c
#include <windows.h>

HANDLE CreateFileMapping(
    HANDLE hFile,                    // 文件句柄
    LPSECURITY_ATTRIBUTES lpAttributes, // 安全属性
    DWORD flProtect,                 // 保护标志
    DWORD dwMaximumSizeHigh,         // 大小高位
    DWORD dwMaximumSizeLow,          // 大小低位
    LPCSTR lpName                    // 映射名称
);
```

#### 参数说明
- `hFile`: 
  - `INVALID_HANDLE_VALUE`: 使用系统页面文件
  - 文件句柄: 映射实际文件
- `flProtect`: 保护标志
  - `PAGE_READWRITE`: 读写访问
  - `PAGE_READONLY`: 只读访问
  - `PAGE_WRITECOPY`: 写时复制
- `dwMaximumSizeHigh/Low`: 映射大小
- `lpName`: 共享内存名称

### 2. 映射视图到地址空间
```c
LPVOID MapViewOfFile(
    HANDLE hFileMappingObject,       // 文件映射句柄
    DWORD dwDesiredAccess,           // 访问模式
    DWORD dwFileOffsetHigh,          // 偏移高位
    DWORD dwFileOffsetLow,           // 偏移低位
    SIZE_T dwNumberOfBytesToMap      // 映射字节数
);
```

#### 访问模式
- `FILE_MAP_ALL_ACCESS`: 读写访问
- `FILE_MAP_READ`: 只读访问
- `FILE_MAP_WRITE`: 写访问
- `FILE_MAP_COPY`: 写时复制

### 3. 打开已存在的文件映射
```c
HANDLE OpenFileMapping(
    DWORD dwDesiredAccess,           // 访问模式
    BOOL bInheritHandle,             // 继承标志
    LPCSTR lpName                   // 映射名称
);
```

### 4. 取消映射视图
```c
BOOL UnmapViewOfFile(
    LPCVOID lpBaseAddress           // 基地址
);
```

### 5. 关闭文件映射句柄
```c
BOOL CloseHandle(
    HANDLE hObject                  // 对象句柄
);
```

## 使用示例

### 1. 创建共享内存（服务器端）
```c
#include <windows.h>
#include <stdio.h>

#define SHM_SIZE 1024
#define SHM_NAME "Global\\MySharedMemory"

typedef struct {
    int counter;
    char message[256];
    SYSTEMTIME timestamp;
} SharedData;

int main() {
    HANDLE hMapFile;
    SharedData* pData;
    
    // 创建文件映射对象
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,       // 使用页面文件
        NULL,                       // 默认安全属性
        PAGE_READWRITE,             // 读写访问
        0,                          // 高位大小
        sizeof(SharedData),         // 低位大小
        SHM_NAME                    // 映射名称
    );
    
    if (hMapFile == NULL) {
        printf("CreateFileMapping failed: %d\n", GetLastError());
        return 1;
    }
    
    // 映射视图
    pData = (SharedData*)MapViewOfFile(
        hMapFile,                   // 映射句柄
        FILE_MAP_ALL_ACCESS,        // 读写访问
        0,                          // 高位偏移
        0,                          // 低位偏移
        sizeof(SharedData)           // 映射大小
    );
    
    if (pData == NULL) {
        printf("MapViewOfFile failed: %d\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }
    
    // 使用共享内存
    pData->counter = 0;
    strcpy(pData->message, "Hello from server!");
    GetSystemTime(&pData->timestamp);
    
    printf("Shared memory created and initialized\n");
    printf("Press Enter to exit...\n");
    getchar();
    
    // 清理资源
    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    
    return 0;
}
```

### 2. 连接共享内存（客户端）
```c
#include <windows.h>
#include <stdio.h>

#define SHM_SIZE 1024
#define SHM_NAME "Global\\MySharedMemory"

typedef struct {
    int counter;
    char message[256];
    SYSTEMTIME timestamp;
} SharedData;

int main() {
    HANDLE hMapFile;
    SharedData* pData;
    
    // 打开已存在的文件映射对象
    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,        // 读写访问
        FALSE,                      // 不继承
        SHM_NAME                    // 映射名称
    );
    
    if (hMapFile == NULL) {
        printf("OpenFileMapping failed: %d\n", GetLastError());
        printf("Make sure the server is running\n");
        return 1;
    }
    
    // 映射视图
    pData = (SharedData*)MapViewOfFile(
        hMapFile,                   // 映射句柄
        FILE_MAP_ALL_ACCESS,        // 读写访问
        0,                          // 高位偏移
        0,                          // 低位偏移
        sizeof(SharedData)           // 映射大小
    );
    
    if (pData == NULL) {
        printf("MapViewOfFile failed: %d\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }
    
    // 使用共享内存
    printf("Connected to shared memory\n");
    printf("Counter: %d\n", pData->counter);
    printf("Message: %s\n", pData->message);
    printf("Timestamp: %02d:%02d:%02d\n", 
           pData->timestamp.wHour,
           pData->timestamp.wMinute,
           pData->timestamp.wSecond);
    
    // 修改数据
    pData->counter++;
    strcpy(pData->message, "Hello from client!");
    GetSystemTime(&pData->timestamp);
    
    printf("Data updated\n");
    printf("Press Enter to exit...\n");
    getchar();
    
    // 清理资源
    UnmapViewOfFile(pData);
    CloseHandle(hMapFile);
    
    return 0;
}
```

## 安全考虑

### 1. 安全描述符
```c
SECURITY_ATTRIBUTES sa;
SECURITY_DESCRIPTOR sd;

// 初始化安全描述符
InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);

// 设置DACL为空，允许所有访问
SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

sa.nLength = sizeof(SECURITY_ATTRIBUTES);
sa.lpSecurityDescriptor = &sd;
sa.bInheritHandle = FALSE;

// 使用安全属性创建共享内存
hMapFile = CreateFileMapping(
    INVALID_HANDLE_VALUE,
    &sa,                          // 应用安全属性
    PAGE_READWRITE,
    0,
    sizeof(SharedData),
    SHM_NAME
);
```

### 2. 命名空间
- `"Local\\MySharedMemory"`: 仅当前会话可访问
- `"Global\\MySharedMemory"`: 所有会话可访问（需要服务权限）
- 无名称: 匿名共享内存，只能通过句柄继承

### 3. 权限管理
```c
// 设置特定权限
EXPLICIT_ACCESS ea;
PACL pAcl = NULL;

// 初始化访问控制项
ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
ea.grfAccessPermissions = FILE_MAP_ALL_ACCESS;
ea.grfAccessMode = SET_ACCESS;
ea.grfInheritance = NO_INHERITANCE;
ea.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
ea.Trustee.ptstrName = L"EVERYONE";

// 创建ACL
SetEntriesInAcl(1, &ea, NULL, &pAcl);

// 应用到安全描述符
SetSecurityDescriptorDacl(&sd, FALSE, pAcl, FALSE);
```

## 同步机制

### 1. 互斥体(Mutex)
```c
HANDLE hMutex = CreateMutex(
    NULL,                           // 默认安全属性
    FALSE,                          // 初始不拥有
    L"Global\\MyMutex"              // 互斥体名称
);

// 等待互斥体
WaitForSingleObject(hMutex, INFINITE);

// 访问共享内存
// ...

// 释放互斥体
ReleaseMutex(hMutex);
```

### 2. 信号量(Semaphore)
```c
HANDLE hSemaphore = CreateSemaphore(
    NULL,                           // 默认安全属性
    1,                              // 初始计数
    BUFFER_SIZE,                     // 最大计数
    L"Global\\MySemaphore"          // 信号量名称
);

// 等待信号量
WaitForSingleObject(hSemaphore, INFINITE);

// 访问共享内存
// ...

// 释放信号量
ReleaseSemaphore(hSemaphore, 1, NULL);
```

### 3. 事件(Event)
```c
HANDLE hEvent = CreateEvent(
    NULL,                           // 默认安全属性
    FALSE,                          // 自动重置
    FALSE,                          // 初始无信号
    L"Global\\MyEvent"              // 事件名称
);

// 设置事件
SetEvent(hEvent);

// 等待事件
WaitForSingleObject(hEvent, INFINITE);
```

## 内存映射文件

### 1. 映射实际文件
```c
HANDLE hFile = CreateFile(
    L"data.bin",                    // 文件名
    GENERIC_READ | GENERIC_WRITE,   // 访问模式
    FILE_SHARE_READ | FILE_SHARE_WRITE, // 共享模式
    NULL,                           // 默认安全属性
    OPEN_ALWAYS,                    // 打开或创建
    FILE_ATTRIBUTE_NORMAL,          // 正常属性
    NULL                            // 无模板
);

HANDLE hMapFile = CreateFileMapping(
    hFile,                          // 文件句柄
    NULL,                           // 默认安全属性
    PAGE_READWRITE,                 // 读写访问
    0,                              // 高位大小
    0,                              // 使用文件大小
    NULL                            // 匿名映射
);
```

### 2. 稀疏文件映射
```c
// 设置文件指针位置
SetFilePointer(hFile, LARGE_SIZE, NULL, FILE_BEGIN);

// 设置文件尾
SetEndOfFile(hFile);

// 创建稀疏文件
DWORD bytesReturned;
DeviceIoControl(hFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &bytesReturned, NULL);
```

## 性能优化

### 1. 大页面支持
```c
// 使用大页面
HANDLE hMapFile = CreateFileMapping(
    INVALID_HANDLE_VALUE,
    NULL,
    PAGE_READWRITE | SEC_LARGE_PAGES, // 启用大页面
    0,
    LARGE_SIZE,
    NULL
);
```

### 2. 预分配内存
```c
// 预分配内存区域
VirtualAlloc(
    NULL,                           // 地址
    SIZE,                          // 大小
    MEM_RESERVE | MEM_COMMIT,       // 预留并提交
    PAGE_READWRITE                  // 保护属性
);
```

### 3. 内存对齐
```c
SYSTEM_INFO si;
GetSystemInfo(&si);

// 按页面大小对齐
SIZE_T alignedSize = ((size + si.dwPageSize - 1) / si.dwPageSize) * si.dwPageSize;
```

## 调试和监控

### 1. 任务管理器
- 查看进程的内存使用情况
- 监控共享内存对象

### 2. Process Explorer
- 详细的进程和句柄信息
- 共享内存使用情况

### 3. 性能计数器
```c
// 获取性能数据
PDH_HQUERY hQuery;
PDH_HCOUNTER hCounter;

PdhOpenQuery(NULL, 0, &hQuery);
PdhAddCounter(hQuery, L"\\Memory\\Shared Bytes", 0, &hCounter);
PdhCollectQueryData(hQuery);

// 获取计数器值
PDH_FMT_COUNTERVALUE value;
PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &value);
```

### 4. WinDbg调试
```
!handle 0 0f <shared_memory_handle>
!address <shared_memory_address>
!vprot <shared_memory_address>
```

## 常见问题

### 1. 访问被拒绝
```c
// 检查权限
DWORD error = GetLastError();
if (error == ERROR_ACCESS_DENIED) {
    printf("Access denied. Check permissions.\n");
}
```

### 2. 内存不足
```c
if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
    printf("Not enough memory.\n");
}
```

### 3. 名称冲突
```c
// 使用唯一名称
char uniqueName[256];
sprintf(uniqueName, "Global\\MySharedMemory_%d", GetCurrentProcessId());
```

## 最佳实践

1. **错误处理**: 始终检查API返回值和GetLastError()
2. **资源清理**: 确保关闭所有句柄
3. **同步机制**: 使用适当的同步原语
4. **安全考虑**: 设置适当的访问权限
5. **命名约定**: 使用有意义的名称避免冲突
6. **性能优化**: 考虑内存对齐和大页面
7. **调试支持**: 添加日志和错误信息

## 与Unix的对比

| 特性 | Windows | Unix |
|------|---------|------|
| API风格 | Win32 API | POSIX/SysV |
| 命名机制 | 对象名称 | 文件路径/键值 |
| 安全模型 | ACL/安全描述符 | 权限位 |
| 同步机制 | Mutex/Event/Semaphore | 信号量/互斥锁 |
| 管理工具 | Process Explorer | ipcs/ipcrm |
| 页面支持 | 大页面支持 | 大页面支持 |
