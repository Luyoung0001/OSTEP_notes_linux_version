# 基于锁的并发数据结构
通过锁可以使数据结构线程安全（thread safe）。当然，具体如何加锁决定了该数据结构的正确性和效率？

## 关键问题：如何给数据结构加锁？

对于特定数据结构，如何加锁才能让该结构功能正确？进一步，如何对该数据结构加锁，能够保证高性能，让许多线程同时访问该结构，即并发访问（concurrently）？

这里涉及的其实就是线程安全的数据结构,它们都可以被安全的访问,每次访问都可以看成一组事物.

## 并发计数器

```C
1    typedef struct  counter_t {
2        int            value;
3        pthread_mutex_t lock;
4    } counter_t;
5
6    void init(counter_t *c) {
7        c->value = 0;
8        Pthread_mutex_init(&c->lock,  NULL);
9    }
10
11   void increment(counter_t *c) {
12       Pthread_mutex_lock(&c->lock);
13       c->value++;
14       Pthread_mutex_unlock(&c->lock);
15   }
16
17   void decrement(counter_t *c) {
18       Pthread_mutex_lock(&c->lock);
19       c->value--;
20       Pthread_mutex_unlock(&c->lock);
21   }
22
23   int get(counter_t *c) {
24       Pthread_mutex_lock(&c->lock);
25       int rc = c->value;
26       Pthread_mutex_unlock(&c->lock);
27       return rc;
28   }
```
这个简单的计数器很简单,但是有性能问题.

线程越多,性能越差,这是因为获取锁和释放锁开销很大.

理想情况下，你会看到多处理上运行的多线程就像单线程一样快。达到这种状态称为完美扩展（perfect scaling）。虽然总工作量增多，但是并行执行后，完成任务的时间并没有增加。

因此说一个程序扩展性差,是说它与完美拓展差距很大,达不到拓展的预期.

## 可拓展的计数器

令人吃惊的是，关于如何实现可扩展的计数器，研究人员已经研究了多年。更令人吃惊的是，最近的操作系统性能分析研究表明，可扩展的计数器很重要。没有可扩展的计数，一些运行在Linux上的工作在多核机器上将遇到严重的扩展性问题。

懒惰计数器通过多个局部计数器和一个全局计数器来实现一个逻辑计数器，其中每个CPU核心有一个局部计数器。具体来说，在4个CPU的机器上，有4个局部计数器和1个全局计数器。除了这些计数器，还有锁：每个局部计数器有一个锁，全局计数器有一个。

局部值会定期转移给全局计数器，方法是获取全局锁，让全局计数器加上局部计数器的值，然后将局部计数器置零.

是阈值S为1024时懒惰计数器的性能。性能很高，4个处理器更新400万次的时间和一个处理器更新100万次的几乎一样。

![Alt text](image.png)

完善后的懒惰计数器就如`lazy_counter.c`一样,它通过开辟 4 个线程,每个线程单独执行,然后各个线程计数,触发条件之后,将自己的报数合并到总的计数上.

这里的关键是,如果 CPU 的个数少于 4 个,性能可能就开始下降了.因为这意味着这至少有一个线程将会进入竟态.而另一个计数器在做一次运算之后,可能会丢失计数器,这样就又会把时间浪费在切换上下文上了.

## 并发链表

接下来看一个更复杂的数据结构，链表。同样，我们从一个基础实现开始。简单起见，我们只关注链表的插入操作.

在文件`list.c`中实现了一个 4 线程的线程安全的链表.

从代码中可以看出，代码插入函数入口处获取锁，结束时释放锁。如果malloc失败（在极少的时候），会有一点小问题，在这种情况下，代码在插入失败之前，必须释放锁。

事实表明，这种异常控制流容易产生错误。最近一个Linux内核补丁的研究表明，有40%都是这种很少发生的代码路径（实际上，这个发现启发了我们自己的一些研究，我们从Linux文件系统中移除了所有内存失败的路径，得到了更健壮的系统[S+11]）。

所以这种严苛的编程习惯增强了整个系统的健壮性.

事实上,我们也没有必要让这把锁将那么大的临界区锁起来,我们可以只把这个锁放在几行关键的地方,让队列的插入变成一个逻辑上的原子操作即可.
![Alt text](image-1.png)

## 拓展链表

尽管我们有了基本的并发链表，但又遇到了这个链表扩展性不好的问题。研究人员发现的增加链表并发的技术中，有一种叫作过手锁（hand-over-hand locking，也叫作锁耦合，lock coupling.

原理也很简单。每个节点都有一个锁，替代之前整个链表一个锁。遍历链表的时候，首先抢占下一个节点的锁，然后释放当前节点的锁。

从概念上说，过手锁链表有点道理，它增加了链表操作的并发程度。

但是实际上，在遍历的时候，每个节点获取锁、释放锁的开销巨大，很难比单锁的方法快。

即使有大量的线程和很大的链表，这种并发的方案也不一定会比单锁的方案快。也许某种杂合的方案（一定数量的节点用一个锁）值得去研究。

### 提示：更多并发不一定更快

如果方案带来了大量的开销（例如，频繁地获取锁、释放锁），那么高并发就没有什么意义。如果简单的方案很少用到高开销的调用，通常会很有效。增加更多的锁和复杂性可能会适得其反。话虽如此，有一种办法可以获得真知：实现两种方案（简单但少一点并发，复杂但多一点并发），测试它们的表现。毕竟，你不能在性能上作弊。结果要么更快，要么不快。

## 并发队列

总有一个标准的方法来创建一个并发数据结构：添加一把大锁。对于一个队列，我们将跳过这种方法，假定你能弄明白。

以下这个例子,将会展示另一种实现并发队列的方法.

```C
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
        newHead; // 相当于踢出队列(访问不到),但是还真是存在,让其充当头结点,这个技巧很不错,缺点是头结点不停地变化
    pthread_mutex_unlock(&queue->headLock);
    free(temp);
    return 0;
}
```
仔细研究这段代码，你会发现有两个锁，一个负责队列头，另一个负责队列尾。这两个锁使得入队列操作和出队列操作可以并发执行，因为入队列只访问tail锁，而出队列只访问head锁。

## 并发散列表

讨论最后一个应用广泛的并发数据结构，散列表. 我们只关注不需要调整大小的简单散列表:

```C
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
```

本例的散列表使用我们之前实现的并发链表，性能特别好。每个散列桶（每个桶都是一个链表）都有一个锁，而不是整个散列表只有一个锁，从而支持许多并发操作.

![Alt text](image-2.png)

## 建议：避免不成熟的优化（Knuth定律）

实现并发数据结构时，先从最简单的方案开始，也就是加一把大锁来同步。这样做，你很可能构建了正确的锁。如果发现性能问题，那么就改进方法，只要优化到满足需要即可。正如Knuth的著名说法“不成熟的优化是所有坏事的根源。”

## 小结

从计数器到链表队列，最后到大量使用的散列表。同时，我们也学习到：控制流变化时注意获取锁和释放锁；增加并发不一定能提高性能；有性能问题的时候再做优化。关于最后一点，避免不成熟的优化（premature optimization），对于所有关心性能的开发者都有用。我们让整个应用的某一小部分变快，却没有提高整体性能，其实没有价值。

如果是很频繁的触发,那么更应该注意优化带来的开销.













