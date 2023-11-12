#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
// 模拟四位哲学家就餐,使得他们能顺利就餐,并且不会有人挨饿.
// 拿到餐具的人,吃完饭后就立即放下餐具,让给其他人(看起来不是那么卫生)

// 设置餐具

sem_t forks[5];
// 初始化这些叉子
void forks_init(){
    for(int i = 0; i < 5; i++){
        sem_init(&forks[i],0,1);
    }
}

// 假设哲学家编号为 p,则他拿的叉子(左右两个叉子)编号应该为:
int left(int p){
    return p;
}
int right(int p){
    return (p+1)%5;
}

// 拿到叉子,准备吃饭
void getforks(int p){
    if(p == 4){
        sem_wait(&forks[right(p)]);
        sem_wait(&forks[left(p)]);
        // 拿到了两个叉子就能吃饭了
        printf("%d号哲学家开始就餐.\n",p);
        sleep(2);
    }else{
    sem_wait(&forks[left(p)]);
    sem_wait(&forks[right(p)]);
    // 拿到了两个叉子就能吃饭了
    printf("%d号哲学家开始就餐.\n",p);
    sleep(2);
    }
}
// 放下餐具
void putforks(int p){
    printf("%d号哲学家完成就餐,并放下了叉子.\n",p);
    sem_post(&forks[left(p)]);
    sem_post(&forks[right(p)]);
}

// 就餐
void *eat(void *arg){
    int p = *((int*)arg);
    getforks(p);
    putforks(p);
    return NULL;
}


int main(){
    forks_init();
    pthread_t p[5];
    int i0 = 0;
    pthread_create(&p[0],NULL,eat,&i0);
    int i1 = 1;
    pthread_create(&p[1],NULL,eat,&i1);
    int i2 = 2;
    pthread_create(&p[2],NULL,eat,&i2);
    int i3 = 3;
    pthread_create(&p[3],NULL,eat,&i3);
    int i4 = 4;
    pthread_create(&p[4],NULL,eat,&i4);



    for(int i = 0; i < 5; i++){
        pthread_join(p[i],NULL);
        sem_close(&forks[i]);
    }
    return 0;
}