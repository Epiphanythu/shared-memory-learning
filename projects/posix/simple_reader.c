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
    int last_counter = 0;
    
    printf("POSIX共享内存读取程序启动...\n");
    printf("等待写入程序启动...\n");
    
    // 1. 打开共享内存对象
    shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        printf("请先运行写入程序\n");
        exit(EXIT_FAILURE);
    }
    
    // 2. 映射共享内存到进程地址空间
    shared_data = mmap(NULL, sizeof(shared_data_t), 
                      PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap failed");
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    // 3. 打开信号量
    semaphore = sem_open(SEM_NAME, 0);
    if (semaphore == SEM_FAILED) {
        perror("sem_open failed");
        munmap(shared_data, sizeof(shared_data_t));
        close(shm_fd);
        exit(EXIT_FAILURE);
    }
    
    printf("成功连接到共享内存，开始读取数据...\n");
    printf("----------------------------------------\n");
    
    // 4. 读取数据循环
    while (1) {
        // 获取信号量
        if (sem_wait(semaphore) == -1) {
            perror("sem_wait failed");
            break;
        }
        
        // 检查是否有新数据
        if (shared_data->counter > last_counter) {
            last_counter = shared_data->counter;
            
            printf("读取到新数据 #%d:\n", shared_data->counter);
            printf("  消息: %s\n", shared_data->message);
            printf("  写入进程PID: %d\n", shared_data->writer_pid);
            printf("  时间戳: %s", ctime(&shared_data->timestamp));
            printf("----------------------------------------\n");
            
            // 如果读取到第10条消息，退出循环
            if (shared_data->counter >= 10) {
                // 释放信号量
                sem_post(semaphore);
                break;
            }
        }
        
        // 释放信号量
        if (sem_post(semaphore) == -1) {
            perror("sem_post failed");
            break;
        }
        
        // 等待一段时间再检查
        usleep(100000); // 100ms
    }
    
    printf("数据读取完成\n");
    
    // 5. 清理资源
    sem_close(semaphore);
    munmap(shared_data, sizeof(shared_data_t));
    close(shm_fd);
    
    printf("读取程序退出\n");
    return 0;
}
