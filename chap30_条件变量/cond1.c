#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
int done = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
void thread_exit() {
    pthread_mutex_lock(&m);
    done = 1;
    pthread_cond_signal(&c);
    pthread_mutex_unlock(&m);
}
void *child(void *arg) {
    printf("child: hello, world!\n");
    thread_exit();
    return NULL;
}
void thread_join() {
    pthread_mutex_lock(&m);
    while (done == 0) {
        pthread_cond_wait(&c, &m);
    }
    pthread_mutex_unlock(&m);
}
int main() {
    printf("parent: hello, son!\n");
    pthread_t p;
    pthread_create(&p, NULL, child, NULL);
    thread_join();
    printf("parent: wtf?!\n");
    return 0;
}