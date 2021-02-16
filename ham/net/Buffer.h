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
    static const  size_t kInitialSize = 1024;
    static const  size_t kCheapPrepend = 8;
    static const char kCRLF[];

    Buffer();
    void swap(const Buffer& rhs);
    
    size_t readableBytes() const { return writeIndex - readIndex ;}
    size_t writableBytes() const { return buffer_.size() - writeIndex; }
    size_t prependableBytes() const { return readIndex; }

    // peek but not read the data away
    const char* peek() const { return begin() + readIndex ;}
    int8_t peekInt8() const;
    int16_t peekInt16() const;
    int32_t peekInt32() const;
    int32_t peekInt64() const;

    int8_t readInt8() const;
    int16_t readInt16() const;
    int32_t readInt32() const;
    int32_t readInt64() const;

    // move the readIndex (after read)
    void retrieve(size_t len);
    void retrieveInt8() { retrieve(sizeof(int8_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt64() { retrieve(sizeof(int64_t)); }

    void hasWritten(size_t len) { writeIndex += len; }
    void append(const char* data, size_t len);
    void append(const std::string& str);

private:
    // 注意到这里，典型的const重载
    char* begin() { return &(*buffer_.begin()); }
    const char* begin() const { return &(*buffer_.begin()); }
    
};


}
}

#endif // __BUFFER_H__