#ifndef __CHARGEN_H__
#define __CHARGEN_H__

#include "ham/net/TcpServer.h"
#include "ham/net/Callbacks.h"
#include <memory>
#include <string>

class EventLoop;
class ChargenServer
{
private:
    EventLoop* loop_;
    std::unique_ptr<TcpServer> server_;
    
};

#endif // __CHARGEN_H__