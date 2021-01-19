# Ham
C++11 server based on muduo


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
