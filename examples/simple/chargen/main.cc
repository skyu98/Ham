#include "chargen.h"
#include "ham/net/EventLoop.h"
#include "ham/net/CurrentThread.h"

using namespace ham;
using namespace ham::net;

int main()
{
    // LOG_LEVEL_INFO;
    INFO("pid = {}", CurrentThread::tid());
    EventLoop loop;
    InetAddress listenAddr(8888);
    ChargenServer server(&loop, listenAddr, true);
    server.start();

    loop.loop();
}