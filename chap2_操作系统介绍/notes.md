它需要 3 条指令：
- 一条将计数器的值从内存加载到寄存器，一条将其递增，另一条将其保存回内存。
- 因为这 3 条指令并不是以原子方式（atomically）执行（所有的指令一次性执行）的，所以奇怪的事情可能会发生。

我的理解是:
- A 线程拿到 555,将 555 放进寄存器中,然后+1,放回到内存中 556
- B 线程在  A线程将555变成556之前 也将555拿出来+1,放回到内存中 556
- 这就导致了它们浪费了一次操作,所以总的次数一定会小于 double

假设是原子操作,会发生什么呢?

这意味着不管哪一个线程运行一次,计数器都加一.这样就是完美的 double!
程序`thread1.c`就完成了这样的事.