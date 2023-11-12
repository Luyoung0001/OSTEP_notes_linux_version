#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#define MAX 10

int buffer[MAX];
int fill = 0;
int use = 0;

// 生产行为
void put(int value) {
    buffer[fill] = value;
    fill = (fill + 1) % MAX;
}

// 消费行为
int get() {
    int temp = buffer[use];
    use = (use + 1) % MAX;
    return temp;
}
sem_t *empty;
sem_t *full;

// 生产者
void *producer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        sem_wait(empty);
        put(i);
        sem_post(full);
    }
    return NULL;
}
// 消费者
void *consumer(void *arg) {
    int value = 0;
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        sem_wait(full);
        value = get();
        sem_post(empty);
        printf("消费:%d\n", value);
    }
    return NULL;
}
int main() {
    // sem_init(&empty, 0, MAX);
    // sem_init(&full, 0, 0);
    // 以上写法已经废弃
    empty = sem_open("empty", O_CREAT, S_IRUSR | S_IWUSR, MAX);
    full = sem_open("full", O_CREAT, S_IRUSR | S_IWUSR, 0);
    pthread_t p1, p2;
    int loop = 10;
    pthread_create(&p1, NULL, producer, &loop);
    pthread_create(&p2, NULL, consumer, &loop);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    sem_close(empty);
    sem_close(full);

    sem_unlink("empty");
    sem_unlink("full");

    return 0;
}