# 常见并发问题

最近的研究集中在一些其他类型的常见并发缺陷（即非死锁缺陷）。

## 有哪些类型的缺陷
对于第一类非死锁的缺陷，我们通过该研究的例子来讨论。对于第二类死锁缺陷，我们讨论人们在阻止、避免和处理死锁上完成的大量工作。

## 非死锁缺陷

我们现在主要讨论其中两种：违反原子性（atomicity violation）缺陷和错误顺序（order violation）缺陷。

### 违法原子性缺陷

```C
1    Thread 1::
2    if (thd->proc_info) {
3      ...
4      fputs(thd->proc_info, ...);
5      ...
6    }
7
8    Thread 2::
9    thd->proc_info = NULL;
```

更正式的违反原子性的定义是：“违反了多次内存访问中预期的可串行性（即代码段本意是原子的，但在执行中并没有强制实现原子性）”

在这个方案中，我们只要给共享变量的访问加锁，确保每个线程访问proc_info字段时，都持有锁（proc_info_lock）。

```C
1    pthread_mutex_t proc_info_lock = PTHREAD_MUTEX_INITIALIZER;
2
3    Thread 1::
4    pthread_mutex_lock(&proc_info_lock);
5    if (thd->proc_info) {
6      ...
7      fputs(thd->proc_info, ...);
8      ...
9    }
10    pthread_mutex_unlock(&proc_info_lock);
11
12   Thread 2::
13   pthread_mutex_lock(&proc_info_lock);
14   thd->proc_info = NULL;
15   pthread_mutex_unlock(&proc_info_lock);
```

### 违反顺序缺陷
Lu等人提出的另一种常见的非死锁问题叫作违反顺序（order violation）.

```C
1    Thread 1::
2    void init() {
3        ...
4        mThread = PR_CreateThread(mMain, ...);
5        ...
6    }
7
8    Thread 2::
9    void mMain(...) {
10       ...
11       mState = mThread->State;
12       ...
13   }
```

违反顺序更正式的定义是：“两个内存访问的预期顺序被打破了（即A应该在B之前执行，但是实际运行中却不是这个顺序）”[L+08]。

我们通过强制顺序来修复这种缺陷。正如之前详细讨论的，条件变量（condition variables）就是一种简单可靠的方式，在现代代码集中加入这种同步。

```C
1    pthread_mutex_t mtLock = PTHREAD_MUTEX_INITIALIZER;
2    pthread_cond_t mtCond = PTHREAD_COND_INITIALIZER;
3    int mtInit            = 0;
4
5    Thread 1::
6    void init() {
7       ...
8       mThread = PR_CreateThread(mMain, ...);
9
10      // signal that the thread has been created...
11      pthread_mutex_lock(&mtLock);
12      mtInit = 1;
13      pthread_cond_signal(&mtCond);
14      pthread_mutex_unlock(&mtLock);
15      ...
16   }
17
18   Thread 2::
19   void mMain(...) {
20      ...
21      // wait for the thread to be initialized...
22      pthread_mutex_lock(&mtLock);
23      while (mtInit == 0)
24          pthread_cond_wait(&mtCond,  &mtLock);
25      pthread_mutex_unlock(&mtLock);
26
27      mState = mThread->State;
28      ...
29   }
```

### 非死锁缺陷:小结

Lu等人的研究中，大部分（97%）的非死锁问题是违反原子性和违反顺序这两种。因此，程序员仔细研究这些错误模式，应该能够更好地避免它们。

## 死锁缺陷

除了上面提到的并发缺陷，死锁（deadlock）是一种在许多复杂并发系统中出现的经典问题。例如，当线程1持有锁L1，正在等待另外一个锁L2，而线程2持有锁L2，却在等待锁L1释放时，死锁就产生了。以下的代码片段就可能出现这种死锁：

```C
Thread 1:    Thread 2:
lock(L1);    lock(L2);
lock(L2);    lock(L1);
```

### 为什么发生死锁

- 其中一个原因是在大型的代码库里，组件之间会有复杂的依赖.
- 另一个原因是封装（encapsulation）。软件开发者一直倾向于隐藏实现细节，以模块化的方式让软件开发更容易。然而，模块化和锁不是很契合。

### 产生死锁的条件

- 互斥访问
- 非抢占(还是互斥访问)
- 循环等待(强连通图)
- 持有并等待

破坏任意一个条件,互斥就不会发生.本质上,以上四个条件都是等价的,是相同问题的不同描述.

### 预防

防止让代码运行到上面的四个困境,最有效的就是避免循环等待.

更复杂的系统中不会只有两个锁，锁的全序可能很难做到。因此，偏序（partial ordering）可能是一种有用的方法，安排锁的获取并避免死锁.

### 提示通过锁的地址来强制锁的顺序

当一个函数要抢多个锁时，我们需要注意死锁。比如有一个函数：do_something(mutex t *m1, mutex t *m2)，如果函数总是先抢m1，然后m2，那么当一个线程调用do_something(L1, L2)，而另一个线程调用do_something(L2, L1)时，就可能会产生死锁。为了避免这种特殊问题，聪明的程序员根据锁的地址作为获取锁的顺序。按照地址从高到低，或者从低到高的顺序加锁，do_something()函数就可以保证不论传入参数是什么顺序，函数都会用固定的顺序加锁。具体的代码如下：

```C

if (m1 > m2) { // grab locks in high-to-low address order
  pthread_mutex_lock(m1);
  pthread_mutex_lock(m2);
} else {
  pthread_mutex_lock(m2);
  pthread_mutex_lock(m1);
　
}
// Code assumes that m1 != m2 (it is not the same lock)
```

### 持有并等待
死锁的持有并等待条件，可以通过原子地抢锁来避免。

```C
1    lock(prevention);
2    lock(L1);
3    lock(L2);
4    ...
5    unlock(prevention);
```

### 非抢占

在调用unlock之前，都认为锁是被占有的，多个抢锁操作通常会带来麻烦，因为我们等待一个锁时，同时持有另一个锁。很多线程库提供更为灵活的接口来避免这种情况。具体来说，trylock()函数会尝试获得锁，或者返回−1，表示锁已经被占有。

### 通过调度来避免死锁

除了死锁预防，某些场景更适合死锁避免（avoidance）。我们需要了解全局的信息，包括不同线程在运行中对锁的需求情况，从而使得后续的调度能够避免产生死锁。

### 检查和恢复
最后一种常用的策略就是允许死锁偶尔发生，检查到死锁时再采取行动。举个例子，如果一个操作系统一年死机一次，你会重启系统，然后愉快地（或者生气地）继续工作。如果死锁很少见，这种不是办法的办法也是很实用的。

很多数据库系统使用了死锁检测和恢复技术。死锁检测器会定期运行，通过构建资源图来检查循环。当循环（死锁）发生时，系统需要重启。如果还需要更复杂的数据结构相关的修复，那么需要人工参与。

### 提示：不要总是完美（TOM WEST定律）
Tom West是经典的计算机行业小说《Soul of a New Machine》[K81]的主人公，有一句很棒的工程格言：“不是所有值得做的事情都值得做好”。如果坏事很少发生，并且造成的影响很小，那么我们不应该去花费大量的精力去预防它。当然，如果你在制造航天飞机，事故会导致航天飞机爆炸，那么你应该忽略这个建议.


## 小结

第一种是非常常见的，非死锁缺陷，通常也很容易修复。这种问题包括：

- 违法原子性，即应该一起执行的指令序列没有一起执行；
- 违反顺序，即两个线程所需的顺序没有强制保证。


我们简要地讨论了死锁：为何会发生，以及如何处理。






















