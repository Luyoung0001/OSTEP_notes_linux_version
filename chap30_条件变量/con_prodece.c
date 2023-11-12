#include <pthread.h>
#include <stdio.h>
#include <assert.h>

int buffer;
int count = 0; // 资源为空

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

/***********第一版本**********/
// 生产者
void *producer(void *arg) {
    int loops = *((int *)arg);
    for (int i = 0; i < loops; i++) {
        put(i);
    }
    return NULL;
}
// 消费者
void *consumer(void *arg) {
    while (1) {
        int temp = get();
        printf("消费的值:%d\n", temp);
    }
    return NULL;
}

int main() {
    pthread_t p1, p2;
    int arg = 100;
    pthread_create(&p1, NULL, producer, &arg);
    pthread_create(&p2, NULL, consumer, NULL);
    // 等待两个线程结束
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    return 0;
}