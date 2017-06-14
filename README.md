# tcpstat

`tcpstat`是一个内核模块，用于统计系统`TCP`状态。
模块通过`proc`伪文件系统，提供一个节点`/proc/tcpstat`，内容为系统当前所有`TCP`连接的状态统计。
例子如下：

```
ESTABLISHED : 5
SYN_SENT    : 0
SYN_RECV    : 0
FIN_WAIT1   : 0
FIN_WAIT2   : 0
TIME_WAIT   : 1
CLOSE       : 0
CLOSE_WAIT  : 0
LAST_ACK    : 0
LISTEN      : 14
CLOSING     : 0
```

## 用法

1. 需要先编译；

```
$ cd tcpstat
$ make
```

2. 挂载到内核；

```
$ sudo insmod tcpstat.ko
```

3. 用起来！

```
cat /proc/tcpstat
ESTABLISHED : 5
SYN_SENT    : 0
SYN_RECV    : 0
FIN_WAIT1   : 0
FIN_WAIT2   : 0
TIME_WAIT   : 1
CLOSE       : 0
CLOSE_WAIT  : 0
LAST_ACK    : 0
LISTEN      : 14
CLOSING     : 0
```

## 异议

有人会问，统计`TCP`状态，遍历`/proc/net/tcp`和`/proc/net/tcp6`不行吗？搞那么复杂？

是的，一般情况下遍历上述连个文件，也是可以得到统计结果的。
然而，在**连接数达到百万级**的系统上，会有什么事情发生呢——光读出这两个文件的内容就好几十秒甚至几百秒！
更别说，做文本处理以及统计了！`/proc/net/tcp`的实现方式注定了效率很低很低。

`tcpstat`就不同了，直接在内核里面遍历链表进行统计，避免了文本生成以及与大量数据拷贝。
因此，很快很快~

## 兼容性

本模块为内核`3.16.0`设计，并没有考虑其他版本内核的差异性。
由于内核用于维护`TCP`连接的哈希表实现时有调整，需要在其他版本内核应用，请先验证兼容性。
如需移植，请参考内核源码做对应调整，修改幅度应该不大。
