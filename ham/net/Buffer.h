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
    size_t readIndex_;
    size_t writeIndex_;
public:
    static const  size_t kInitialSize = 1024;
    static const  size_t kCheapPrepend = 8;
    static const char kCRLF[];
    
    Buffer();
    void swap(Buffer& rhs);

    const char* findCRLF() const;
    const char* findCRLF(const char* start) const;
    const char* findEOL() const;
    const char* findEOL(const char* start) const;
    
    size_t readableBytes() const { return writeIndex_ - readIndex_ ;}
    size_t writableBytes() const { return buffer_.size() - writeIndex_; }
    size_t prependableBytes() const { return readIndex_; }

    // peek but not read the data away
    const char* peek() const { return begin() + readIndex_ ;}
    int8_t peekInt8() const;
    int16_t peekInt16() const;
    int32_t peekInt32() const;
    int64_t peekInt64() const;

    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();

    // 使用网络字节序写入一个整数
    void appendInt64(int64_t x);
    void appendInt32(int32_t x);
    void appendInt16(int16_t x);
    void appendInt8(int8_t x);

    // move the readIndex_ (after read)
    void retrieve(size_t len);
    void retrieveInt8() { retrieve(sizeof(int8_t)); }
    void retrieveInt16() { retrieve(sizeof(int16_t)); }
    void retrieveInt32() { retrieve(sizeof(int32_t)); }
    void retrieveInt64() { retrieve(sizeof(int64_t)); }
    void retrieveAll();
    void retrieveUtil(const char* end);
    std::string retrieveAsString(size_t len);
    std::string retrieveAllAsString();

    void ensureWritableBytes(size_t len);
    void shrink(size_t reserve);

    char* beginWrite() { return begin() + writeIndex_; }
    const char* beginWrite() const { return begin() + writeIndex_; }
    void hasWritten(size_t len) { writeIndex_ += len; }

    void append(const char* data, size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void prepend(const void* data, size_t len);

    ssize_t readFd(int fd, int* savedErrno);
private:
    // 注意到这里，典型的const重载
    char* begin() { return &(*buffer_.begin()); }
    const char* begin() const { return &(*buffer_.begin()); }

    void makeSpace(size_t len);
    
};


}
}

#endif // __BUFFER_H__