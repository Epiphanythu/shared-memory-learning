#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <time.h>

#define SHM_NAME "/my_shared_memory"
#define SEM_NAME "/my_semaphore"
#define SHM_SIZE 1024

typedef struct {
    time_t timestamp;
    pid_t writer_pid;
    char message[256];
    int counter;
} shared_data_t;

int main() {
    int shm_fd;
    shared_data_t *shared_data;
    sem_t *semaphore;
    
    printf("POSIX共享内存写入程序启动...\n");
    
    // 1. 创建或打开共享内存对象
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(EXIT_FAILURE);
    }
    
    // 2. 设置共享内存大小
    if (ftruncate(shm_fd, sizeof(shared_data_t)) == -1) {
        perror("ftruncate failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    // 3. 映射共享内存到进程地址空间
    shared_data = mmap(NULL, sizeof(shared_data_t), 
                      PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    // 4. 创建或打开信号量
    semaphore = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (semaphore == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, sizeof(shared_data_t));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("共享内存设置完成，开始写入数据...\n");
    
    // 5. 写入数据循环
    for (int i = 0; i < 10; i++) {
        // 获取信号量
        if (sem_wait(semaphore) == -1) {
            perror("sem_wait failed");
            break;
        }
        
        // 写入共享数据
        shared_data->timestamp = time(NULL);
        shared_data->writer_pid = getpid();
        shared_data->counter = i + 1;
        snprintf(shared_data->message, sizeof(shared_data->message), 
                "这是第%d条消息", i + 1);
        
        printf("写入数据 #%d: %s (PID: %d, 时间: %s", 
               i + 1, shared_data->message, shared_data->writer_pid, 
               ctime(&shared_data->timestamp));
        
        // 释放信号量
        if (sem_post(semaphore) == -1) {
            perror("sem_post failed");
            break;
        }
        
        // 等待1秒
        sleep(1);
    }
    
    printf("数据写入完成，按Enter键清理资源...");
    getchar();
    
    // 6. 清理资源
    sem_close(semaphore);
    sem_unlink(SEM_NAME);
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    
    printf("资源清理完成\n");
    return 0;
}
