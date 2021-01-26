#include "../EventLoop.h"
#include <iostream>

using namespace ham;
using namespace ham::net;
int main()
{
    auto el = EventLoop::create();
    std::cout << "111" << std::endl;
}