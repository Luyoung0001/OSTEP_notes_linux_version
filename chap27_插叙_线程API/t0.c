#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
typedef struct myarg_t {
    int a;
    int b;
} myarg_t;
typedef struct myret_t {
    int a;
    int b;
} myret_t;
void *mythread(void *arg) {
    myarg_t *m = (myarg_t *)arg;
    printf("%d %d\n", m->a, m->b);
    myret_t *r = malloc(sizeof(myret_t));
    r->a = 10;
    r->b = 20;
    // myret_t r;
    // r.a = 10;
    // r.b = 20;
    printf("r 的值:%p\n",r);
    return (void *)r;
}
int main(int argc, char *argv[]) {

    pthread_t p;
    myret_t *m;
    printf("m 的值:%p\n",m);
    printf("m 本身的地址:%p\n",&m);
    myarg_t arg;
    arg.a = 1;
    arg.b = 2;
    pthread_create(&p, NULL, mythread, &arg);
    pthread_join(p, (void**)&m);
    printf("%d %d\n", m->a,m->b);
    printf("m 的值:%p\n",m);
    printf("m 本身的地址:%p\n",&m);
    return 0;
}