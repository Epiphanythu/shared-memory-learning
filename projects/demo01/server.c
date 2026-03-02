#include "common.h"

int main()
{
    struct Shared *ptr;      // 共享内存指针
    sem_t *mutex;            // 信号量指针
    int fd;                  // 文件描述符

    // 删除已存在的共享内存对象（如果存在）
    shm_unlink(SHM_FILE);
    // 创建新的共享内存对象
    fd = shm_open(SHM_FILE, O_RDWR | O_CREAT, 0666);
    // 设置共享内存大小
    ftruncate(fd, sizeof(struct Shared));
    // 映射共享内存到进程地址空间
    ptr = mmap(NULL, sizeof(struct Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    // 关闭文件描述符
    close(fd);

    // 删除已存在的信号量（如果存在）
    sem_unlink(SEM_PATH);
    // 创建新的信号量，初始值为1
    mutex = sem_open(SEM_PATH, O_CREAT, 0666, 1);
    // 关闭信号量
    sem_close(mutex);

    // 等待信号（暂停服务器进程，直到收到终止信号）
    pause();

    return 0;
}
