#include "producer_consumer.h"

int create_shared_memory() {
    int shmid = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    return shmid;
}

int create_semaphore(int key, int initial_value) {
    int semid = semget(key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }
    
    union semun arg;
    arg.val = initial_value;
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        perror("semctl SETVAL failed");
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

void init_buffer(shared_buffer_t *buffer) {
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    buffer->total_produced = 0;
    buffer->total_consumed = 0;
    buffer->start_time = time(NULL);
}

int main(int argc, char *argv[]) {
    int shmid, semid_mutex, semid_empty, semid_full;
    shared_buffer_t *buffer;
    int producer_id;
    int production_count = 20; // 默认生产20个产品
    int production_delay = 1000000; // 默认延迟1秒
    
    // 解析命令行参数
    if (argc >= 2) {
        producer_id = atoi(argv[1]);
    } else {
        producer_id = 1;
    }
    
    if (argc >= 3) {
        production_count = atoi(argv[2]);
    }
    
    if (argc >= 4) {
        production_delay = atoi(argv[3]);
    }
    
    printf("生产者 #%d 启动 (将生产 %d 个产品，延迟 %d 微秒)\n", 
           producer_id, production_count, production_delay);
    
    // 创建或获取共享内存和信号量
    shmid = create_shared_memory();
    semid_mutex = create_semaphore(SEM_KEY_MUTEX, 1);
    semid_empty = create_semaphore(SEM_KEY_EMPTY, BUFFER_SIZE);
    semid_full = create_semaphore(SEM_KEY_FULL, 0);
    
    // 附加共享内存
    buffer = (shared_buffer_t *)shmat(shmid, NULL, 0);
    if (buffer == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    // 初始化缓冲区（如果是第一个生产者）
    sem_wait(semid_mutex);
    if (buffer->start_time == 0) {
        init_buffer(buffer);
        printf("初始化共享缓冲区\n");
    }
    sem_post(semid_mutex);
    
    // 生产循环
    for (int i = 0; i < production_count; i++) {
        product_t product;
        
        // 等待空槽位
        sem_wait(semid_empty);
        
        // 获取互斥锁
        sem_wait(semid_mutex);
        
        // 生产产品
        product.id = buffer->total_produced + 1;
        product.timestamp = time(NULL);
        product.producer_pid = getpid();
        snprintf(product.data, sizeof(product.data), 
                "Producer #%d - Item #%d", producer_id, i + 1);
        
        // 将产品放入缓冲区
        buffer->buffer[buffer->in] = product;
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        buffer->count++;
        buffer->total_produced++;
        
        printf("生产者 #%d 生产了产品 #%d: %s\n", 
               producer_id, product.id, product.data);
        
        // 释放互斥锁
        sem_post(semid_mutex);
        
        // 通知有产品可用
        sem_post(semid_full);
        
        // 生产延迟
        usleep(production_delay);
    }
    
    // 分离共享内存
    if (shmdt(buffer) == -1) {
        perror("shmdt failed");
    }
    
    printf("生产者 #%d 完成，共生产了 %d 个产品\n", producer_id, production_count);
    return 0;
}
