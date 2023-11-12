#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int buffer;
int count = 0; // 资源为空
pthread_cond_t cond_consumer;
pthread_cond_t cond_procedure;
pthread_mutex_t mutex;

// 生产,在 buffer 中放入一个值
void put(int value) {
    assert(count == 0);
    count = 1;
    buffer = value;
}
// 消费,取出 buffer 中的值
int get() {
    assert(count == 1);
    count = 0;
    return buffer;
}

/***********第四版本**********/
// 虽然解决了虚假唤醒
// 假设生产者唤醒了第一个消费者,消费者又唤醒了第二个生产者,第二个生产者
// 之后又睡眠.这样三个线程都在睡眠.
// 核心问题就是,消费者只能唤醒生产者,生产者只能唤醒消费者.
// 生产者
void *producer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        while (count == 1) {
            pthread_cond_wait(&cond_procedure, &mutex);
        }
        put(i);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond_consumer);

    }
    return NULL;
}
// 消费者
void *consumer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);

        while (count == 0) {
            pthread_cond_wait(&cond_consumer, &mutex);
        }
        int temp = get();
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond_procedure);
        printf("消费:%d\n", temp);
    }
    return NULL;
}

int main() {

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_consumer, NULL);
    pthread_cond_init(&cond_procedure, NULL);

    pthread_t p1, p2, p3;
    int arg = 100;
    int arg1 = 50;
    int arg2 = 50;
    pthread_create(&p1, NULL, producer, &arg);
    pthread_create(&p2, NULL, consumer, &arg1);
    pthread_create(&p3, NULL, consumer, &arg2);
    // 等待线程结束
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_consumer);
    pthread_cond_destroy(&cond_procedure);
    return 0;
}