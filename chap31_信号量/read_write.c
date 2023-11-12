#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
// 读取的空间
char space[10] = "hello";

// 读者写者锁
typedef struct _rwlock_t {
    sem_t lock;
    sem_t writelock;
    int readers;
} rwlock_t;

// 初始化
void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    sem_init(&rw->lock,0,1);
    sem_init(&rw->writelock,0,1);
}
// 获得读锁(实际上就不需要上锁,上锁只是为了修改 rw)
void rwlock_acquire_readlock(rwlock_t *rw) {
    sem_wait(&rw->lock);
    rw->readers++;
    if (rw->readers == 1) {
        sem_wait(&rw->writelock); // 第一个读者需要抢占写锁
    }
    sem_post(&rw->lock);
}
// 释放读锁(实际上就不需要上锁,上锁只是为了修改 rw)
void rwlock_release_readlock(rwlock_t *rw) {
    sem_wait(&rw->lock);
    rw->readers--;
    if (rw->readers == 0) {
        sem_post(&rw->writelock); // 最后一个读者需要释放写锁
    }
    sem_post(&rw->lock);
}
// 获得写锁
void rwlock_acquire_writelock(rwlock_t *rw) { sem_wait(&rw->writelock); }
// 释放写锁
void rwlock_release_writelock(rwlock_t *rw) { sem_post(&rw->writelock); }

rwlock_t rwlock;

// 写线程
void *write_t(void *arg) {
    rwlock_acquire_writelock(&rwlock);
    strcpy(space, arg);
    rwlock_release_writelock(&rwlock);
    return NULL;
}

// 读线程
void *read_t(void *arg) {
    rwlock_acquire_readlock(&rwlock);
    printf("%s\n", space);
    rwlock_release_readlock(&rwlock);
    return NULL;
}

int main() {
    rwlock_init(&rwlock);
    // 创建 5 个读线程和 5 个写线程
    pthread_t p_r1, p_r2, p_r3, p_r4, p_r5;
    pthread_t p_w1, p_w2, p_w3, p_w4, p_w5;

    pthread_create(&p_r1, NULL, read_t, NULL);
    pthread_create(&p_r2, NULL, read_t, NULL);
    pthread_create(&p_r3, NULL, read_t, NULL);


    char arg[10] = "1111111";
    pthread_create(&p_w1, NULL, write_t, arg);
    arg[2] = '2';
    pthread_create(&p_w2, NULL, write_t, arg);
    arg[3] = '3';
    pthread_create(&p_w3, NULL, write_t, arg);
    arg[4] = '4';
    pthread_create(&p_w4, NULL, write_t, arg);
    arg[5] = '5';
    pthread_create(&p_w5, NULL, write_t, arg);
    pthread_create(&p_r4, NULL, read_t, NULL);
    pthread_create(&p_r5, NULL, read_t, NULL);

    pthread_join(p_r1, NULL);
    pthread_join(p_r2, NULL);
    pthread_join(p_r3, NULL);
    pthread_join(p_r4, NULL);
    pthread_join(p_r5, NULL);
    
    pthread_join(p_w1, NULL);
    pthread_join(p_w2, NULL);
    pthread_join(p_w3, NULL);
    pthread_join(p_w4, NULL);
    pthread_join(p_w5, NULL);

    sem_close(&rwlock.lock);
    sem_close(&rwlock.writelock);

    sem_unlink("rw_lock");
    sem_unlink("rw_writelock");

    return 0;
}