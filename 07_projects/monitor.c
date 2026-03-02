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

// 获取信号量值（非阻塞）
int get_semaphore_value(int semid) {
    return semctl(semid, 0, GETVAL, NULL);
}

void print_status(shared_buffer_t *buffer) {
    time_t current_time = time(NULL);
    double elapsed_time = difftime(current_time, buffer->start_time);
    
    printf("\n=== 生产者-消费者系统状态 ===\n");
    printf("运行时间: %.1f 秒\n", elapsed_time);
    printf("缓冲区状态: %d/%d (当前/最大容量)\n", buffer->count, BUFFER_SIZE);
    printf("写入位置: %d\n", buffer->in);
    printf("读取位置: %d\n", buffer->out);
    printf("总生产数量: %d\n", buffer->total_produced);
    printf("总消费数量: %d\n", buffer->total_consumed);
    printf("生产速率: %.1f 个/秒\n", 
           buffer->total_produced / (elapsed_time > 0 ? elapsed_time : 1));
    printf("消费速率: %.1f 个/秒\n", 
           buffer->total_consumed / (elapsed_time > 0 ? elapsed_time : 1));
    
    // 显示缓冲区内容概览
    if (buffer->count > 0) {
        printf("\n当前缓冲区内容:\n");
        int index = buffer->out;
        for (int i = 0; i < buffer->count; i++) {
            product_t *product = &buffer->buffer[index];
            char time_str[32];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", 
                     localtime(&product->timestamp));
            printf("  [%d] ID:%d Producer:%d %s\n", 
                   i + 1, product->id, product->producer_pid, time_str);
            index = (index + 1) % BUFFER_SIZE;
        }
    } else {
        printf("\n缓冲区为空\n");
    }
    printf("===============================\n\n");
}

void print_buffer_visualization(shared_buffer_t *buffer) {
    printf("缓冲区可视化: [");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (i == buffer->in && i == buffer->out) {
            if (buffer->count == 0) {
                printf("□");  // 空缓冲区
            } else if (buffer->count == BUFFER_SIZE) {
                printf("■");  // 满缓冲区
            } else {
                printf("◇");  // 读写位置重叠
            }
        } else if (i == buffer->in) {
            printf("→");  // 写入位置
        } else if (i == buffer->out) {
            printf("←");  // 读取位置
        } else if ((i >= buffer->out && i < buffer->out + buffer->count) ||
                   (buffer->out + buffer->count > BUFFER_SIZE && 
                    i < (buffer->out + buffer->count) % BUFFER_SIZE)) {
            printf("■");  // 有数据
        } else {
            printf("□");  // 空
        }
        
        if (i < BUFFER_SIZE - 1) printf(" ");
    }
    printf("]\n");
    printf("图例: ■有数据 □空 →写入 ←读取 ◇读写重叠\n");
}

void monitor_system(int update_interval) {
    int shmid, semid_mutex, semid_empty, semid_full;
    shared_buffer_t *buffer;
    
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
    
    printf("共享内存监控程序启动\n");
    printf("更新间隔: %d 秒\n", update_interval);
    printf("按 Ctrl+C 退出\n\n");
    
    // 监控循环
    while (1) {
        system("clear");  // 清屏
        
        printf("\033[1;32m共享内存监控系统\033[0m\n");
        printf("========================\n");
        
        // 获取信号量状态
        int mutex_val = get_semaphore_value(semid_mutex);
        int empty_val = get_semaphore_value(semid_empty);
        int full_val = get_semaphore_value(semid_full);
        
        printf("信号量状态:\n");
        printf("  互斥锁: %s (值: %d)\n", 
               mutex_val == 1 ? "可用" : "被占用", mutex_val);
        printf("  空槽位: %d 个可用\n", empty_val);
        printf("  满槽位: %d 个可用\n", full_val);
        
        // 获取缓冲区状态
        sem_wait(semid_mutex);
        print_status(buffer);
        print_buffer_visualization(buffer);
        
        // 计算效率统计
        if (buffer->total_produced > 0) {
            double efficiency = (double)buffer->total_consumed / buffer->total_produced * 100;
            printf("系统效率: %.1f%%\n", efficiency);
            
            if (buffer->count == BUFFER_SIZE) {
                printf("\033[1;31m警告: 缓冲区已满！生产者可能阻塞\033[0m\n");
            } else if (buffer->count == 0 && buffer->total_produced > 0) {
                printf("\033[1;33m提示: 缓冲区为空，等待生产者生产\033[0m\n");
            }
        }
        sem_post(semid_mutex);
        
        // 系统信息
        printf("\n系统信息:\n");
        printf("  共享内存ID: %d\n", shmid);
        printf("  当前时间: %s", ctime(&(time_t){time(NULL)}));
        
        sleep(update_interval);
    }
    
    // 分离共享内存（实际上不会到达这里）
    if (shmdt(buffer) == -1) {
        perror("shmdt failed");
    }
}

void print_help() {
    printf("用法: ./monitor [选项]\n");
    printf("选项:\n");
    printf("  -i <秒数>   设置更新间隔（默认2秒）\n");
    printf("  -h          显示帮助信息\n");
    printf("\n示例:\n");
    printf("  ./monitor           # 使用默认2秒间隔\n");
    printf("  ./monitor -i 5      # 使用5秒间隔\n");
}

int main(int argc, char *argv[]) {
    int update_interval = 2;  // 默认2秒更新间隔
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            print_help();
            return 0;
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            update_interval = atoi(argv[++i]);
            if (update_interval <= 0) {
                printf("错误: 更新间隔必须大于0\n");
                return 1;
            }
        }
    }
    
    monitor_system(update_interval);
    return 0;
}
