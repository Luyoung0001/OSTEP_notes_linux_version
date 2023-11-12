#include <assert.h>
#include <pthread.h>
#include <stdio.h>

int buffer;
int count = 0; // 资源为空
pthread_cond_t cond;
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

/***********第三版本**********/
// 存在虚假唤醒,导致断言失败
// 生产者
void *producer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        if (count == 1) {
            pthread_cond_wait(&cond, &mutex);
        }
        put(i);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }
    return NULL;
}
// 消费者
void *consumer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        pthread_mutex_lock(&mutex);
        if (count == 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        int temp = get();
        pthread_mutex_unlock(&mutex);
        printf("消费:%d\n", temp);
        pthread_cond_signal(&cond); // 在解锁之后发出信号
    }
    return NULL;
}

int main() {

    // 初始化互斥锁和条件变量
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_t p1, p2, p3;
    int arg = 100;
    int arg1 = 50;
    int arg2 = 50;
    pthread_create(&p1, NULL, producer, &arg);
    pthread_create(&p2, NULL, consumer, &arg1);
    pthread_create(&p3, NULL, consumer, &arg2);
    // 等待两个线程结束
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    // 销毁互斥锁和条件变量
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    return 0;
}