#include "producer_consumer.h"

// 获取已存在的共享内存
int get_shared_memory() {
    int shmid = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        printf("请先启动生产者程序\n");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

// 获取已存在的信号量
int get_semaphore(int key) {
    int semid = semget(key, 1, 0666);
    if (semid == -1) {
        perror("semget failed");
        printf("请先启动生产者程序\n");
        exit(EXIT_FAILURE);
    }
    return semid;
}

void sem_wait(int semid) {
    struct sembuf op = {0, -1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("sem_wait failed");
        exit(EXIT_FAILURE);
    }
}

void sem_post(int semid) {
    struct sembuf op = {0, 1, 0};
    if (semop(semid, &op, 1) == -1) {
        perror("sem_post failed");
        exit(EXIT_FAILURE);
    }
}

void print_consumed_product(product_t *product, int consumer_id) {
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
             localtime(&product->timestamp));
    
    printf("消费者 #%d 消费了产品 #%d: %s\n", 
           consumer_id, product->id, product->data);
    printf("  生产时间: %s\n", time_str);
    printf("  生产者PID: %d\n", product->producer_pid);
}

int main(int argc, char *argv[]) {
    int shmid, semid_mutex, semid_empty, semid_full;
    shared_buffer_t *buffer;
    int consumer_id;
    int consumption_count = 20; // 默认消费20个产品
    int consumption_delay = 1500000; // 默认延迟1.5秒
    int consumed = 0;
    
    // 解析命令行参数
    if (argc >= 2) {
        consumer_id = atoi(argv[1]);
    } else {
        consumer_id = 1;
    }
    
    if (argc >= 3) {
        consumption_count = atoi(argv[2]);
    }
    
    if (argc >= 4) {
        consumption_delay = atoi(argv[3]);
    }
    
    printf("消费者 #%d 启动 (将消费 %d 个产品，延迟 %d 微秒)\n", 
           consumer_id, consumption_count, consumption_delay);
    
    // 获取共享内存和信号量
    shmid = get_shared_memory();
    semid_mutex = get_semaphore(SEM_KEY_MUTEX);
    semid_empty = get_semaphore(SEM_KEY_EMPTY);
    semid_full = get_semaphore(SEM_KEY_FULL);
    
    // 附加共享内存
    buffer = (shared_buffer_t *)shmat(shmid, NULL, 0);
    if (buffer == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    printf("成功连接到共享缓冲区，等待产品...\n");
    
    // 消费循环
    while (consumed < consumption_count) {
        // 等待产品可用
        sem_wait(semid_full);
        
        // 获取互斥锁
        sem_wait(semid_mutex);
        
        // 检查是否有产品可消费
        if (buffer->count > 0) {
            product_t product = buffer->buffer[buffer->out];
            
            // 从缓冲区取出产品
            buffer->out = (buffer->out + 1) % BUFFER_SIZE;
            buffer->count--;
            buffer->total_consumed++;
            consumed++;
            
            // 打印消费的产品信息
            print_consumed_product(&product, consumer_id);
            
            // 释放互斥锁
            sem_post(semid_mutex);
            
            // 通知有空槽位可用
            sem_post(semid_empty);
            
            // 消费延迟
            usleep(consumption_delay);
        } else {
            // 没有产品可消费，释放锁继续等待
            sem_post(semid_mutex);
            // 重新放回一个full信号量，因为我们需要重新等待
            sem_post(semid_full);
            usleep(100000); // 短暂等待避免忙等待
        }
    }
    
    // 分离共享内存
    if (shmdt(buffer) == -1) {
        perror("shmdt failed");
    }
    
    printf("消费者 #%d 完成，共消费了 %d 个产品\n", consumer_id, consumed);
    return 0;
}
