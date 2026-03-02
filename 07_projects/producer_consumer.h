#ifndef PRODUCER_CONSUMER_H
#define PRODUCER_CONSUMER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>

// 共享内存键值
#define SHM_KEY 0x1234
#define SEM_KEY_MUTEX 0x5678
#define SEM_KEY_EMPTY 0x9ABC
#define SEM_KEY_FULL 0xDEF0

// 环形缓冲区大小
#define BUFFER_SIZE 10
#define MAX_MESSAGE_LEN 256

// 产品结构体
typedef struct {
    int id;                     // 产品ID
    time_t timestamp;           // 生产时间戳
    pid_t producer_pid;         // 生产者PID
    char data[MAX_MESSAGE_LEN]; // 产品数据
} product_t;

// 环形缓冲区结构体
typedef struct {
    product_t buffer[BUFFER_SIZE]; // 缓冲区
    int in;                       // 写入位置
    int out;                      // 读取位置
    int count;                    // 当前产品数量
    int total_produced;           // 总生产数量
    int total_consumed;           // 总消费数量
    time_t start_time;            // 开始时间
} shared_buffer_t;

// 信号量操作结构体
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// 函数声明
int create_shared_memory();
int create_semaphore(int key, int initial_value);
void sem_wait(int semid);
void sem_post(int semid);
void init_buffer(shared_buffer_t *buffer);
void print_status(shared_buffer_t *buffer);
void cleanup_resources(int shmid, int semid_mutex, int semid_empty, int semid_full);

#endif // PRODUCER_CONSUMER_H
