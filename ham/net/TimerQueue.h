#ifndef __TIMERQUEUE_H__
#define __TIMERQUEUE_H__
#include "base/Timestamp.h"
#include "net/Callbacks.h"
#include <boost/noncopyable.hpp>
#include <memory>
#include <vector>
#include <map>
#include <set>

namespace ham
{
namespace net
{

class EventLoop;
class Timer;
class TimerId;
class Channel;

class TimerQueue : public boost::noncopyable
{
public:
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId addTimer(const TimerCallback& cb, Timestamp when, double interval);
    void cancelTimer(TimerId id);

private:

    typedef std::pair<Timestamp, std::shared_ptr<Timer>> timeEntry;
    typedef std::set<timeEntry> TimerList;
    //typedef std::pair<std::shared_ptr<Timer>, int64_t> ActiveTimer;

    void handleRead(Timestamp alarmTime);
    void addTimerInLoop(std::shared_ptr<Timer> timer);
    void cancelTimerInLoop(TimerId);
    void insert(std::shared_ptr<Timer> timer);
    std::vector<timeEntry> getExpired();

    EventLoop* loop_;
    const int alarmFd_;
    std::unique_ptr<Channel> alarmChannel_;
    bool earliestChanged_;
    bool callingExpiredTimers_;
    TimerList timerList_;
    std::set<std::shared_ptr<Timer>> cancelingTimers_;
};
}
}
#endif // __TIMERQUEUE_H__