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
#define SHM_SIZE 1024

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
    
    printf("System V共享内存写入程序启动...\n");
    
    // 1. 创建共享内存段
    shmid = shmget(SHM_KEY, sizeof(shared_data_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    
    // 2. 附加共享内存到进程地址空间
    shared_data = (shared_data_t *)shmat(shmid, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("shmat failed");
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    
    // 3. 创建信号量集
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    
    // 4. 初始化信号量值为1
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL failed");
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    
    // 5. 初始化共享内存
    sem_wait(semid);
    shared_data->counter = 0;
    shared_data->ready = 0;
    sem_post(semid);
    
    printf("共享内存设置完成，开始写入数据...\n");
    printf("共享内存ID: %d, 信号量ID: %d\n", shmid, semid);
    
    // 6. 写入数据循环
    for (int i = 0; i < 10; i++) {
        // 获取信号量
        sem_wait(semid);
        
        // 写入共享数据
        shared_data->timestamp = time(NULL);
        shared_data->writer_pid = getpid();
        shared_data->counter = i + 1;
        snprintf(shared_data->message, sizeof(shared_data->message), 
                "System V消息 #%d", i + 1);
        shared_data->ready = 1;  // 标记数据准备好
        
        printf("写入数据 #%d: %s (PID: %d, 时间: %s", 
               i + 1, shared_data->message, shared_data->writer_pid, 
               ctime(&shared_data->timestamp));
        
        // 释放信号量
        sem_post(semid);
        
        // 等待1秒
        sleep(1);
    }
    
    printf("数据写入完成，按Enter键清理资源...");
    getchar();
    
    // 7. 清理资源
    sem_wait(semid);
    shared_data->ready = 0;  // 通知读取程序结束
    sem_post(semid);
    
    // 等待一段时间确保读取程序能收到结束信号
    sleep(1);
    
    // 分离共享内存
    if (shmdt(shared_data) == -1) {
        perror("shmdt failed");
    }
    
    // 删除共享内存段
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl IPC_RMID failed");
    }
    
    // 删除信号量集
    if (semctl(semid, 0, IPC_RMID, NULL) == -1) {
        perror("semctl IPC_RMID failed");
    }
    
    printf("资源清理完成\n");
    return 0;
}
