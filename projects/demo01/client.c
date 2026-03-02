#include "common.h"

int main(int argc, char **argv)
{
    struct Shared *ptr;      // 共享内存指针
    struct stat buf;         // 文件状态信息
    sem_t *mutex;            // 信号量指针
    int fd;                  // 文件描述符
    int nloop;               // 循环次数
    int i;                   // 循环变量

    // 打开共享内存
    fd = shm_open(SHM_FILE, O_RDWR, 0);
    // 获取共享内存大小
    fstat(fd, &buf);
    // 映射共享内存到进程地址空间
    ptr = mmap(NULL, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // 关闭文件描述符
    close(fd);

    // 打开信号量
    mutex = sem_open(SEM_PATH, 0);
    // 获取循环次数参数
    nloop = atoi(argv[1]);

    // 主循环：访问共享内存
    for (i = 0; i < nloop; i++)
    {
        // 等待信号量（获取锁）
        sem_wait(mutex);
        // 访问共享数据并递增计数器
        printf("pid %d: %d\n", getpid(), ptr->count++);
        // 释放信号量（释放锁）
        sem_post(mutex);
    }

    return 0;
}
