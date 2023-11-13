#include <stdio.h>
#include <pthread.h>

// 自己定义信号量
typedef struct {
    int value;
    pthread_cond_t cond;
    pthread_mutex_t lock;
}zem_t;

// 初始化
void zem_init(zem_t *zem,int value){
    zem->value = value;
    pthread_cond_init(&zem->cond,NULL);
    pthread_mutex_init(&zem->lock,NULL);
}

// wait
void zem_wait(zem_t *zem){
    pthread_mutex_lock(&zem->lock);
    while(zem->value <= 0){
        pthread_cond_wait(&zem->cond,&zem->lock);
    }
    zem->value--;
    pthread_mutex_unlock(&zem->lock);
}

// post
void *zem_post(zem_t *zem){
    pthread_mutex_lock(&zem->lock);
    zem->value++;
    pthread_cond_signal(&zem->cond);
    pthread_mutex_unlock(&zem->lock);
}
