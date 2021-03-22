[toc]

# Ham
C++11 server based on muduoß

# Base 

## 1.Atomic
之前muduo是采用gcc的原子操作函数：
```cpp
T get()
  {
    return __sync_val_compare_and_swap(&value_, 0, 0); // 原子操作：获取value_（等于0设置为0，也是获取value_）
  }

T getAndAdd(T x)
{
return __sync_fetch_and_add(&value_, x);    // 原子操作：获取value_，然后加上给定的X
}
```
现在直接使用C++11的Atomic模版类。常用的类型都有封装，可以直接使用。

## 2.Timestamp
这里主要是对时间的获取进行了修改，采用更方便的chrono库。其余的计算操作大同小异。

## 3.Exception
Exception继承自std::exception，在产生异常时，生成包含异常信息和调用函数栈信息的异常对象，并对外抛出。
这里使用了demangle函数来解码，使结果更易理解。
注意不同平台```::backTrace()```返回的函数栈名格式不同，需要区分。

## 4.CountDownLatch 
用C++11标准的mutex 和 cv 替换了之前自己实现的类。
lock_guard() 比 unique_lock()效率高，后者是为了与wait()搭配使用。

## 5.BlockingQueue
注意实现右值引用版本；因为其底层容器都重载了右值引用版本。

取任务时也可以使用std::move()，使用移动构造函数来构造，因为之后不会再需要他。

## 6.Singleton
这里实例不再作为静态成员变量，而是作为局部静态对象，在getInstance()中构造；只初始化一次，生命期与程序相同。

线程单例，不再使用pthread_key来实现，而是使用*thread_local*标识符来限定。

## 7.threadPool
替换了锁、move等；没有大的改变。
take()的时候会阻塞；被唤醒时有两种可能：
* 1.队列有新任务；此时可以正常pop_front()取任务；
* 2.isRunning为False了；此时队列是空的，不能再pop_front()取任务；
所以唤醒时一定要判断queue的状态，来看是不是假唤醒。

## 8.Epoller
```cpp
typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;
```
struct epoll_event里的data是个共用体Union，共享同一片内存，所以只能写一个成员，否则数据会被覆盖；导致ptr不能指向有效的channel。

# net

## 1.EventLoop
```EventLoop```的本质是一个IO线程。我们笼统地分析，一个IO线程至少需要以下部件：
* 1.```Epoller```:用于关注套接字（```listen_fd```、```conn_fd```）
* 2.```vector<Functor>```:在处理完每个回调函数之后，做一些额外的工作（如取消关注、修改关注、销毁连接等等）
* 3.```mutex```:上述的容器是可以跨线程来添加任务的，所以我们需要一把```Mutex```，来管理线程间的同步
* 4.```wakeup_fd```:有时我们需要IO线程处理任务，但它正阻塞在```epoll_wait()```上，这时就需要通过```wakeup_fd```来唤醒IO线程

除此之外，在某些场景下，我们需要IO线程在某个特定的时刻去完成某些任务。因此，我们需要加入一个```TimerQueue```部件，来管理所有的定时任务。此部分详见```TimerQueue```。

由于这里有了线程所属的概念，所以对于每个函数，我们都要考虑其可不可以跨线程调用：如果不可以，则一定加上assert来验证。

## 2.Channel
回忆C语言写的简单Reactor中，每个fd上树时，我们通过下面的结构体来保存信息:
```cpp
typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;	/* Epoll events */
  epoll_data_t data;	/* User data variable */
} __EPOLL_PACKED;
```

这样肉眼可见有三个缺点：
* 1. ```epoll_data_t```是Union，即只能保存一种信息，否则会被覆盖；显然我们需要
```void *ptr```来保存回调函数指针，那么fd则无法被保存。
* 2. 在半同步半异步模型中，不止一个```EventLoop```，那么一个fd我们还需要保存其所属线程，原来的结构体无法实现。
* 3. 在实际情况下，epoll返回的事件并不一定就是关注的事件（可能是EPOLLHUP之类的），所以我们需要同时知道一个fd关注的事件和返回的事件。

所以我们使用一个Channel对象来对fd进行封装，它具有：
* 1. ```loop_```:该fd所属```EventLoop```
* 2. callbacks:即不同情况下应该调用的回调函数对象
* 3. ```event_```&```revent```:即关注的事件和返回的事件

这就是最基础的```Channel```构架。需要注意的是，```Channel```只是对fd进行封装，它并不拥有fd，即它不负责fd的生命周期。

## 3.TcpClient
### 1.Connector
只负责连接的建立，```connectChannel```也是只存在此期间
```socket()```创建fd -> ```connect()``` -> epoller检查可写/Error -> 连接成功 -> newConnCallback -> 交给```TcpClient```创建新```TcpConnection```


# Http
对于每一个```TcpConnection```，有一个```context```上下文对象；当该连接有Http包到来，```context```则会对该包进行解析，并且将解析结果存放在自己的```HttpRequest```成员中。

## 1.HttpRequest
一个简单的Http请求报文如下：

    GET /simple.html HTTP/1.1<CRLF>       ----- 请求行 
    Accept: text/html<CRLF>               -- 头部
    Accept-Language: zh-cn<CRLF>             
    Accept-Encoding: gzip, deflate<CRLF>  -- 头部 
    User-Agent: Mozilla/4.0<CRLF>            
    Host: localhost:8080<CRLF>               
    Connection: Keep-Alive<CRLF>          -- 头部
    <CRLF>                                ----- 空白行表示头部的结束
    // body                               ----- 接下来的内容是正文部分

一个Http请求包分为下列部分：
* 1.Method:GET/POST...
* 2.path
* 3.Version:Http 1.1/ 1.0
* 4.headers:保存在```map<filed, value>```中
* 5.body

所有的对象都由```context```解析请求包得来.

## 2.HttpContext
这里使用一个```state```来表示当前的解析状态。
* 对于request line：分割出Method、path、Version，并且保存。
* 对于headers：分割出每一行```field: value```，交给```HttpRequest```保存
* 对于body：暂时未实现

每个状态都处理完后，整个包也就处理完毕并保存了。

## 3.HttpResponse

---

BenchMarks   
具体见benchmarks文件夹