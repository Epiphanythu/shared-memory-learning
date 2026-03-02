#ifndef _COMMON_H_
#define _COMMON_H_
// 没有头文件保护的话，common.h 的内容会被处理多次，导致编译错误。

// 标准库头文件
#include <stdio.h>      // 标准输入输出函数
#include <stdlib.h>     // 标准库函数（内存分配、进程控制等）
#include <string.h>     // 字符串处理函数
#include <unistd.h>     // UNIX标准函数（文件操作、进程控制等）
#include <fcntl.h>      // 文件控制选项定义

// 共享内存和信号量相关头文件
#include <semaphore.h>  // POSIX信号量函数
#include <sys/mman.h>   // 内存管理函数（mmap等）
#include <sys/stat.h>   // 文件状态函数（fstat等）

// 共享内存文件名
#define SHM_FILE    "/shm_file"
// 信号量名称
#define SEM_PATH    "/sem_mmap"

// 共享内存数据结构
struct Shared
{
    int count;         // 共享计数器
};

#endif
