#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUMCPUS 4
#define COUNT_PER_THREAD 10000000
int th;
typedef struct counter_t {
    int global;                     // global count
    pthread_mutex_t glock;          // global lock
    int local[NUMCPUS];             // local count (per cpu)
    pthread_mutex_t llock[NUMCPUS]; // ... and locks
    int threshold;                  // update frequency
} counter_t;

void init(counter_t *c, int threshold) {
    c->threshold = threshold;

    c->global = 0;
    pthread_mutex_init(&c->glock, NULL);
    // 初始化 4 个锁
    int i;
    for (i = 0; i < NUMCPUS; i++) {
        c->local[i] = 0;
        pthread_mutex_init(&c->llock[i], NULL);
    }
}
// 更新数字
void update(counter_t *c, int threadID, int amt) {
    for (int i = 0; i < COUNT_PER_THREAD; i++) {
        pthread_mutex_lock(&c->llock[threadID]);
        c->local[threadID] += amt;
        if (c->local[threadID] >= c->threshold) {
            pthread_mutex_lock(&c->glock);
            c->global += c->local[threadID];
            pthread_mutex_unlock(&c->glock);
            c->local[threadID] = 0;
        }
        pthread_mutex_unlock(&c->llock[threadID]);
    }
}

int get(counter_t *c) {
    pthread_mutex_lock(&c->glock);
    int val = c->global;
    pthread_mutex_unlock(&c->glock);
    return val;
}

void *thread_function(void *arg) {
    counter_t *counter = (counter_t *)arg;
    update(counter, th, 1);
    return NULL;
}

int main() {
    counter_t *counter = (counter_t *)malloc(sizeof(counter_t));
    init(counter, 10000);

    pthread_t p1, p2, p3, p4;
    th = 0;
    pthread_create(&p1, NULL, thread_function, counter);
    th = 1;
    pthread_create(&p2, NULL, thread_function, counter);
    th = 2;
    pthread_create(&p3, NULL, thread_function, counter);
    th = 3;
    pthread_create(&p4, NULL, thread_function, counter);

    // 等待线程完成
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);

    printf("Global count: %d\n", get(counter));

    free(counter);

    return 0;
}
