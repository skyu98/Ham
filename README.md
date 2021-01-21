# Ham
C++11 server based on muduo

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
