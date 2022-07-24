# SimplewebServer
simple chat system by socket implemented by C++
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

