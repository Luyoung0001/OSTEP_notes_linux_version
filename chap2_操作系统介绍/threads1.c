#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int counter = 0;
pthread_mutex_t mutex; // 互斥锁
int loops;

void *worker(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        // 加锁
        pthread_mutex_lock(&mutex);
        counter++;
        // 解锁
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: threads <value> \n");
        exit(1);
    }

    loops = atoi(argv[1]);

    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);

    pthread_t p1, p2;
    printf("Initial value : %d\n",counter);

    pthread_create(&p1, NULL, worker, NULL);
    pthread_create(&p2, NULL, worker, NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    // 销毁互斥锁
    pthread_mutex_destroy(&mutex);

    printf("Final value: %d\n", counter);
    return 0;
}
