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
sem_t *mutex;

// 生产者
void *producer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {

        sem_wait(empty);
        sem_wait(mutex);
        put(i);
        sem_post(mutex);
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
        sem_wait(mutex);
        value = get();
        sem_post(mutex);
        sem_post(empty);

        printf("消费:%d\n", value);
    }
    return NULL;
}
int main() {
    empty = sem_open("empty", O_CREAT, S_IRUSR | S_IWUSR, MAX);
    full = sem_open("full", O_CREAT, S_IRUSR | S_IWUSR, 0);
    mutex = sem_open("mutex", O_CREAT, S_IRUSR | S_IWUSR, 1);
    pthread_t p1, p2, p3, p4;
    int loop1 = 3;
    int loop2 = 3;
    pthread_create(&p1, NULL, producer, &loop1);
    pthread_create(&p2, NULL, producer, &loop1);
    pthread_create(&p3, NULL, consumer, &loop2);
    pthread_create(&p4, NULL, consumer, &loop2);

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);

    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");

    return 0;
}