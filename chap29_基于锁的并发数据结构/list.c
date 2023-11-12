#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// 定义链表结点
typedef struct node_t {
    int key;
    struct node_t *next;
} node_t;

// 定义一个链表
typedef struct list_t {
    node_t *head;
    pthread_mutex_t lock; // 锁
} list_t;

// 链表初始化
void listInit(list_t *list) {
    list->head = NULL;
    pthread_mutex_init(&list->lock, NULL);
}
// 定义参数
typedef struct arg_t {
    list_t *list;
    int key;
} arg_t;
// 插入
int listInsert(list_t *list, int key) {
    pthread_mutex_lock(&list->lock);
    node_t *newNode = (node_t *)malloc(sizeof(node_t));
    if (newNode == NULL) {
        printf("内存分配失败!\n");
        pthread_mutex_unlock(&list->lock);
        return -1; // 失败
    }
    // 赋值并插入到链表,头插法,其实就是加了个锁....
    newNode->key = key;
    newNode->next = list->head;
    list->head = newNode;
    pthread_mutex_unlock(&list->lock);
    return 0; // 成功
}

// 查找
int listLookUp(list_t *list, int target) {
    pthread_mutex_lock(&list->lock);
    node_t *cur = list->head;
    while (cur != NULL) {
        if (cur->key == target) {
            pthread_mutex_unlock(&list->lock);
            return 0;
        }
        cur = cur->next;
    }
    pthread_mutex_unlock(&list->lock);
    return -1;
}

void *thread_func_insert(void *arg) {
    arg_t *a = (arg_t *)arg;
    listInsert(a->list, a->key);
    return NULL;
}
int main() {
    // 放在栈吧,无所谓
    list_t list;
    listInit(&list);
    pthread_t p1, p2, p3, p4;

    arg_t a1, a2, a3, a4;
    a1.list = &list;
    a1.key = 1;
    a2.list = &list;
    a2.key = 2;
    a3.list = &list;
    a3.key = 3;
    a4.list = &list;
    a4.key = 4;

    // 启动四个线程
    pthread_create(&p1, NULL, thread_func_insert, (void *)&a1);
    pthread_create(&p2, NULL, thread_func_insert, (void *)&a2);
    pthread_create(&p3, NULL, thread_func_insert, (void *)&a3);
    pthread_create(&p4, NULL, thread_func_insert, (void *)&a4);
    // 等待结束
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);

    // 打印这个链表
    node_t *cur = list.head;
    while (cur != NULL) {
        printf("%d ", cur->key);
        cur = cur->next;
    }

    return 0;
}