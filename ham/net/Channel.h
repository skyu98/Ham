#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include "base/Timestamp.h"
#include "base/Log.h"
#include "net/Epoller.h"

#include <memory>
#include <functional>


namespace ham
{
namespace net
{

class EventLoop;

class Channel  //: public std::enable_shared_from_this<Channel>
{
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

public:
    enum status
    {
        kNew,       // 0
        kAdded,     // 1
        kDismissed,   // 2
        n_status
    };

public:
    Channel(EventLoop* loop, int fd);
    ~Channel();
    
    int getFd() const{ return fd_;}
    int getEvent() const { return event_; }
    int getStatus() const { return status_; }
    void setStatus(Channel::status newStatus) { status_ = newStatus; }
    void setRevent(uint32_t revent) { revent_ = revent; }

    /* set callbacks which is called when different revents come */
    void setReadCallback(const ReadEventCallback& cb) { readCb_ = cb; }
    void setWriteCallback(const EventCallback& cb) { writeCb_ = cb; }
    void setCloseCallback(const EventCallback& cb) { closeCb_ = cb; }
    void setErrorCallback(const EventCallback& cb) { errorCb_ = cb; }

    /* change the state of event_ */
    void enableReading() { event_ |= kReadEvent_; update(); }
    void enableWriting() { event_ |= kWriteEvent_; update(); }
    // disableReading() is not needed
    void disableWriting() { event_ &= ~kWriteEvent_; update(); }
    void disableAll() { event_ = kNoneEvent_; update(); }

    bool isWriting() const { return event_ & kWriteEvent_; }
    bool isNoneEvent() const { return event_ == kNoneEvent_; }
    void remove();

    void tie(const std::shared_ptr<void>& obj);
    void handleEvent(Timestamp retTime); 

    EventLoop* ownerLoop() const { return loop_; }

    std::string eventsToString() const { return eventsToString(fd_, event_); }
    std::string reventsToString() const  { return eventsToString(fd_, revent_); }
    
private:
    static std::string eventsToString(int fd, int event);

    void update();  // tell loop I'm ready, please update me.
    void handleEventWithGuard(Timestamp);

    EventLoop* loop_;  // 循环引用。裸指针就行，实在不行就weak_ptr<>
    const int fd_;
    int event_;
    int revent_; 
    Channel::status status_;
    bool isHandlingEvent_;

    std::weak_ptr<void> tie_;
    bool tied_;

    static const int kReadEvent_ = EPOLLIN | EPOLLPRI;
    static const int kWriteEvent_ = EPOLLOUT;
    static const int kNoneEvent_ = 0;

    // Callbacks
    ReadEventCallback readCb_;
    EventCallback writeCb_;
    EventCallback closeCb_;
    EventCallback errorCb_;

};
}
}

#endif // __CHANNEL_H__