#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// 信号量操作结构体
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

typedef struct {
    time_t timestamp;
    pid_t writer_pid;
    char message[256];
    int counter;
    int ready;  // 标志位，表示数据是否准备好
} shared_data_t;

// 等待信号量
void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

// 释放信号量
void sem_post(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

int main() {
    int shmid, semid;
    shared_data_t *shared_data;
    int last_counter = 0;
    
    printf("System V共享内存读取程序启动...\n");
    printf("等待写入程序启动...\n");
    
    // 1. 获取共享内存段
    shmid = shmget(SHM_KEY, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        printf("请先运行写入程序\n");
        exit(EXIT_FAILURE);
    }
    
    // 2. 附加共享内存到进程地址空间
    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    // 3. 获取信号量集
    semid = semget(SEM_KEY, 1, 0666);
    if (semid == -1) {
        perror("semget failed");
        shmdt(shared_data);
        exit(EXIT_FAILURE);
    }
    
    printf("成功连接到共享内存，开始读取数据...\n");
    printf("共享内存ID: %d, 信号量ID: %d\n", shmid, semid);
    printf("----------------------------------------\n");
    
    // 4. 读取数据循环
    while (1) {
        // 获取信号量
        sem_wait(semid);
        
        // 检查是否有新数据
        if (shared_data->counter > last_counter) {
            last_counter = shared_data->counter;
            
            printf("读取到新数据 #%d:\n", shared_data->counter);
            printf("  消息: %s\n", shared_data->message);
            printf("  写入进程PID: %d\n", shared_data->writer_pid);
            printf("  时间戳: %s", ctime(&shared_data->timestamp));
            printf("  准备状态: %s\n", shared_data->ready ? "就绪" : "未就绪");
            printf("----------------------------------------\n");
            
            // 如果读取到第10条消息，准备退出
            if (shared_data->counter >= 10) {
                printf("检测到数据传输完成\n");
                sem_post(semid);
                break;
            }
        }
        
        // 检查写入程序是否已结束
        if (shared_data->ready == 0 && shared_data->counter > 0) {
            printf("检测到写入程序已结束\n");
            sem_post(semid);
            break;
        }
        
        // 释放信号量
        sem_post(semid);
        
        // 等待一段时间再检查
        usleep(100000); // 100ms
    }
    
    printf("数据读取完成\n");
    
    // 5. 清理资源
    if (shmdt(shared_data) == -1) {
        perror("shmdt failed");
    }
    
    printf("读取程序退出\n");
    return 0;
}
