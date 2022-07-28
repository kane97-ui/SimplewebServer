





# C++ Linux socket网络通信实现

* 服务端：服务器端先初始化socket，然后与端口绑定，对端口进行监听，调用accept阻塞，等待客户端连接。
* socket() -> bind() -> listen() -> accept()
* 客户端：客户端先初始化socket，然后与服务端连接，服务端监听成功则连接建立完成
* socket() -> connect()

## TCP编程的一般步骤

| **TCP编程的服务器端一般步骤是**                         | TCP编程的客户端一般步骤是：                              |
| ------------------------------------------------------- | -------------------------------------------------------- |
| 1、创建一个socket，用函数socket()；                     | 1、创建一个socket，用函数socket()；                      |
| 2、设置socket属性，用函数setsockopt(); * 可选           | 2、设置socket属性，用函数setsockopt();* 可选             |
| 3、绑定IP地址、端口等信息到socket上，用函数bind();      | 3、绑定IP地址、端口等信息到socket上，用函数bind();* 可选 |
| 4、开启监听，用函数listen()；                           | 4、设置要连接的对方的IP地址和端口等属性；                |
| 5、接收客户端上来的连接，用函数accept()；               | 5、连接服务器，用函数connect()；                         |
| 6、收发数据，用函数send()和recv()，或者read()和write(); | 6、收发数据，用函数send()和recv()，或者read()和write();  |
| 7、关闭网络连接；                                       | 7、关闭网络连接；                                        |
| 8、关闭监听；                                           |                                                          |



## 设计模式

### 单线程I/O阻塞

accept(), recv(), send()函数都是阻塞的，所以调用的时候主进程会阻塞，去等待I/O。所以基于这种设计模式的话，一个聊天系统必须等收到信息，才能发送，如果没有收到信息，主进程会一直等待.

### 多线程I/O阻塞

这部分代码是在socket_multithread文件中实现的，主要是针对对每个客户端连接分配一个线程，实现多个连接之间的并行，但每一个连接也是阻塞的。服务端对每个新连接都单独起一个线程去处理，主线程继续阻塞在accept上等待新连接。每当accept获取到新的connfd后，把这个connfd交给新的线程去处理。

```cpp
listenfd = socket(); // 初始化监听套接字
bind();   // 绑定监听套接字和服务端地址
listen(listenfd); // 监听
while(1) {
  if ((clientfd=accept()) >= 0) {
    // 如果返回 大于0，代表有新连接产生
    // 启动新的线程去处理这个连接 主线程继续while循环等待新的连接
    new_thread(clientfd);
  }
}
```



### 基于select的I/O多路复用

Select: 最初解决I/O阻塞的问题，通过结构体fd_set来告诉内核监听多少个描述符文件，该结构体被称为描述符文件，有数组来维持哪些描述符位置被置为，通过轮寻来查找会否有描述符要被处理。

select模型是Windows sockets中最常见的IO模型。它利用select函数实现IO 管理。通过对select函数的调用，应用程序可以判断套接字是否存在数据、能否向该套接字写入据。对于accept、connect、read、write等系统调用，实际上都属于慢系统调用，他可能会永远阻塞直到套接字上发生 可读\可写 事件。事实上，通常不希望一直阻塞直到IO就绪，而应该等待IO就绪之后再通知我们过来处理。解决基本C/S模型中，accept()、recv()、send()阻塞的问题，以及C/S模型需要创建大量线程，客户端过多就会增加服务器运行压力。

```cpp
fd_set rfds;  //套接字描述符集合 
FD_ZERO(&rfds);
int maxfd = sclient;
int retval = 0;
FD_SET(sclient, &rfds);
FD_SET(0, &rfds);
struct timeval tv;
tv.tv_sec = 10;//设置倒计时时间
tv.tv_usec = 0;
int select (
  int nfds, //要检测的文件描述符中最大的fd+1 [1024]
  fd_set* readfds, //读集合
  fd_set* writefds, // 写集合
  fd_set* exceptfds, //异常集合 [NULL]
  struct timeval* timeout // [1] NULL永久阻塞 [2]当检测到fd变化的时候返回
  );//返回的是可以调用的套接字
```

### 基于select的I/O多路复用及多线程

此前事在select基础上进行的单线程I/O多路复用，从而实现多客户端连接的非阻塞。现在将accept，send，recv分别在不同线程中运行，实现多线程的目的是为了提高处理能力，减少等待时间，多线程可以并发执行，即可以同时对三个函数进行处理，处理起来会快很多。

```cpp
thread t1(Accpt);
thread t2(GetData);
thread t3(sendData);
t1.join();
t2.join();
t3.join();
```

### 基于Epoll的I/O多路复用

轮寻查找所有文件描述符效率不高，使服务器并发能力受限。因此Epoll采用只返回状态变化的文件描述符，解决了轮寻的瓶颈。

epoll原理

```cpp
int epoll_create(int size);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
int epoll_wait(int epfd, struct epoll_event *event, int maxevents, int timeout)
```

首先创建一个epoll对象，使用epill_ctl对这个对象进行操作，把需要监控的描述添加进去，这些描述将会以epoll_event结构体的形式组成红黑树，接着阻塞在epoll_wait, 加入大循环，当某个有fd上有事件发生，内核会把其对应的结构体放入到一个链表，返回有事件发生的链表。

创建epoll对象（epoll_create)

```cpp
//调用epoll_create()的时候我们会创建这个结构的对象
struct eventpoll {
	ep_rb_tree rbr;      //ep_rb_tree是个结构，所以rbr是结构变量，这里代表红黑树的根；
	int rbcnt;
	
	LIST_HEAD( ,epitem) rdlist;    //rdlist是结构变量，这里代表双向链表的根；
	/*	这个LIST_HEAD等价于下边这个 
		struct {
			struct epitem *lh_first;
		}rdlist;
	*/
	int rdnum; //双向链表里边的节点数量（也就是有多少个TCP连接来事件了）
 
	int waiting;
 
	pthread_mutex_t mtx; //rbtree update
	pthread_spinlock_t lock; //rdlist update
	
	pthread_cond_t cond; //block for event
	pthread_mutex_t cdmtx; //mutex for cond	
};
```

* rbr结构成员：代表一颗红黑树的根节点[刚开始指向空],把rbr理解成红黑树的根节点的指针
* 红黑树，用来保存  键【数字】/值【结构】，能够快速的通过你给key，把整个的键/值取出来: 这里的key是socokid，而value则是event信息
* rdlist结构成员：代表 一个双向链表的表头指针
* 双向链表：从头访问/遍历每个元素特别快next。
* 总结：创建了一个eventpoll结构对象，被系统保存起来。rbr成员被初始化成指向一颗红黑树的根【有了一个红黑树】
  rdlist成员被初始化成指向一个双向链表的根【有了双向链表】阻塞等待的时长

<img width="1059" alt="image" src="https://user-images.githubusercontent.com/62796730/180988307-79c198ea-b669-4d68-a745-bba5a68ac279.png">

### select+线程池
线程池的实现参考 
