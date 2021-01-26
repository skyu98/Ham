#ifndef __CHANNEL_H__
#define __CHANNEL_H__
#include <memory>

namespace ham
{
namespace net
{
class EventLoop;

class Channel : public std::enable_shared_from_this<Channel>
{
public:
    Channel(std::shared_ptr<EventLoop>);
    
private:
    std::shared_ptr<EventLoop> loop_;  // 循环引用。裸指针就行，实在不行就weak_ptr<>
};
}
}

#endif // __CHANNEL_H__