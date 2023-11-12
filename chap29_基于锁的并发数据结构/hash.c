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
#define BUCKETS 101
typedef struct hash_t {
    list_t lists[BUCKETS];
} hash_t;

void hashInit(hash_t *H) {
    for (int i = 0; i < BUCKETS; i++) {
        listInit(&H->lists[i]);
    }
}

// 插入
int insertHash(hash_t *H, int key) {
    // 根据插入的值确定一个桶
    int bucket = key % BUCKETS;
    // 然后再桶里面插入这个值
    return listInsert(&H->lists[bucket], key);
}
// 查询
int lookUpHash(hash_t *H, int key) {
    // 根据插入的值找到这个桶
    int bucket = key % BUCKETS;
    // 在这个桶里面查找
    return listLookUp(&H->lists[bucket], key);
}