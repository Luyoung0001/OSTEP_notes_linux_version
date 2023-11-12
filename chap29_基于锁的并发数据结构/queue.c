#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
// 定义节点类型
typedef struct node_t {
    int value;
    struct node_t *next;
} node_t;

// 定义一个队列
typedef struct queue_t {
    node_t *head;
    node_t *tail;
    pthread_mutex_t headLock;
    pthread_mutex_t tailLock;
} queue_t;

// 初始化一个队列
void queueInit(queue_t *queue) {
    // 创建一个空的节点,然后让队头和队尾都指向它
    node_t *temp = (node_t *)malloc(sizeof(node_t));
    temp->next = NULL;
    queue->head = queue->tail = temp;
    // 然后初始化两个锁
    pthread_mutex_init(&queue->headLock, NULL);
    pthread_mutex_init(&queue->tailLock, NULL);
}
// 进队列
void enQueue(queue_t *queue, int value) {
    node_t *tmp = (node_t *)malloc(sizeof(node_t));
    if (tmp == NULL) {
        printf("内存分配失败!\n");
        return;
    }
    tmp->value = value;
    tmp->next = NULL;
    // 访问队尾,直接事物化
    pthread_mutex_lock(&queue->tailLock);
    // 尾插法
    queue->tail->next = tmp;
    queue->tail = tmp;
    pthread_mutex_unlock(&queue->tailLock);
}
// 删除元素
int deQueue(queue_t *queue, int *value) {
    pthread_mutex_lock(&queue->headLock);
    // 用临时变量指向头结点,而不是第一个元素
    node_t *temp = queue->head;
    // 用 newHead 指向要删除的元素
    node_t *newHead = temp->next;
    if (newHead == NULL) {
        // 空列为空
        pthread_mutex_unlock(&queue->headLock);
        return -1; // 失败
    }
    *value = newHead->value;
    queue->head =
        newHead; // 相当于踢出队列(访问不到),但是还真是存在,让其充当头结点
    pthread_mutex_unlock(&queue->headLock);
    free(temp);
    return 0;
}

int main() { return 0; }