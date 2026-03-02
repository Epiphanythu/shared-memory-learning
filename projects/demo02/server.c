#include "common.h"

int main()
{
    struct Shared *ptr;      // 共享内存指针
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

    // 初始化共享内存中的信号量
    // 参数：1 - 表示信号量在进程间共享，1 - 初始值为1
    sem_init(&ptr->mutex, 1, 1);

    // 暂停服务器进程，直到收到终止信号
    pause();

    return 0;
}
