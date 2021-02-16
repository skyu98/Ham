#ifndef __BUFFER_H__
#define __BUFFER_H__
#include "base/copyable.h"
#include <vector>
#include <string>

namespace ham
{
namespace net
{
class Buffer : public ham::copyable
{
private:
    /* data */
    std::vector<char> buffer_;
    size_t readIndex;
    size_t writeIndex;
public:
    const static size_t kInitialSize = 1024;
    const static size_t kCheapPrepend = 8;

    Buffer();
    
    size_t readable() const { return writeIndex - readIndex ;}
    size_t writable() const { return buffer_.size() - writeIndex; }
    
};


}
}

#endif // __BUFFER_H__