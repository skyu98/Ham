#include "net/Buffer.h"
#include "net/Endian.h"
#include "net/SocketOps.h"
#include "base/Types.h"
#include <assert.h>
#include <cstring>
#include <algorithm>
#include <sys/uio.h>

namespace ham
{
    namespace net
    {
        const char Buffer::kCRLF[]= "\r\n";

        Buffer::Buffer()
            : buffer_(kInitialSize + kCheapPrepend),
              readIndex_(kCheapPrepend), 
              writeIndex_(kCheapPrepend)
        {
            assert(readableBytes() == 0);
            assert(writableBytes() == kInitialSize);
            assert(prependableBytes() == kCheapPrepend);
        }
        
        void Buffer::swap(Buffer& rhs) 
        {
            std::swap(buffer_, rhs.buffer_);
            std::swap(readIndex_, rhs.readIndex_);
            std::swap(writeIndex_, rhs.writeIndex_);
        }
        
        const char* Buffer::findCRLF() const
        {
            return findCRLF(peek());
        }
        
        const char* Buffer::findCRLF(const char* start) const
        {
            assert(peek() <= start);
            assert(start <= beginWrite());
            const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF + 2);
            return crlf == beginWrite() ? nullptr : crlf;
        }
        
        const char* Buffer::findEOL() const
        {
            return findEOL(peek());
        }
        
        const char* Buffer::findEOL(const char* start) const
        {
            assert(peek() <= start);
            assert(start <= beginWrite());
            const void* eol = ::memchr(start, '\n', beginWrite() - start);
            return static_cast<const char*>(eol);
        }
        
        int8_t Buffer::peekInt8() const
        {
            assert(readableBytes() >= sizeof(int8_t));
            int8_t num = *peek();  // one byte, no need to NetWorkToHost
            return num;
        }
        
        int16_t Buffer::peekInt16() const
        {
            assert(readableBytes() >= sizeof(int16_t));
            int16_t be16 = 0;
            ::memcpy(&be16, peek(), sizeof(int16_t));
            return sockets::networkToHost16(be16);
        }
        
        int32_t Buffer::peekInt32() const
        {
            assert(readableBytes() >= sizeof(int32_t));
            int16_t be32 = 0;
            ::memcpy(&be32, peek(), sizeof(int32_t));
            return sockets::networkToHost16(be32);
        }
        
        int32_t Buffer::peekInt64() const
        {
            assert(readableBytes() >= sizeof(int64_t));
            int16_t be64 = 0;
            ::memcpy(&be64, peek(), sizeof(int64_t));
            return sockets::networkToHost16(be64);
        }

        void Buffer::appendInt64(int64_t x)
        {
            int64_t be64 = sockets::hostToNetwork64(x);
            append(&be64, sizeof(be64));
        }

        void Buffer::appendInt32(int32_t x)
        {
            int32_t be32 = sockets::hostToNetwork32(x);
            append(&be32, sizeof(be32));
        }

        void Buffer::appendInt16(int16_t x)
        {
            int16_t be16 = sockets::hostToNetwork16(x);
            append(&be16, sizeof(be16));
        }

        void Buffer::appendInt8(int8_t x)
        {
            append(&x, sizeof(x));
        }
        
        int8_t Buffer::readInt8() 
        {
            int8_t res = peekInt8();
            retrieveInt8();
            return res;
        }
        
        int16_t Buffer::readInt16() 
        {
            int16_t res = peekInt16();
            retrieveInt16();
            return res;
        }
        
        int32_t Buffer::readInt32() 
        {
            int32_t res = peekInt32();
            retrieveInt32();
            return res;
        }
        
        int64_t Buffer::readInt64() 
        {
            int64_t res = peekInt64();
            retrieveInt64();
            return res;
        }
        
        void Buffer::retrieve(size_t len) 
        {
            assert(len <= readableBytes());
            if(len < readableBytes()) 
            {
                readIndex_ += len;
            }
            else
            {
                retrieveAll();
            }
        }
        
        void Buffer::retrieveAll() 
        {
            // 全部读完，read和write都回到起点
            readIndex_ = kCheapPrepend; 
            writeIndex_ = kCheapPrepend; 
            std::cout << "retrieveAll()!!! WriteIndex at " << writeIndex_ << std::endl;
            std::cout << "retrieveAll()!!! ReadIndex at " << readIndex_ << std::endl;
        }
        
        void Buffer::retrieveUtil(const char* end) 
        {
            assert(peek() <= end);
            assert(end <= beginWrite());
            retrieve(end - peek());
        }
        
        std::string Buffer::retrieveAsString(size_t len) 
        {
            assert(len <= readableBytes());
            std::string res(peek(), len);
            retrieve(len);
            return res;
        }
        
        std::string Buffer::retrieveAllAsString() 
        {
            return retrieveAsString(readableBytes());
        }
        
        void Buffer::ensureWritableBytes(size_t len) 
        {
            if(writableBytes() < len)
            {
                makeSpace(len);
            }
            assert(writableBytes() >= len);
        }
        
        void Buffer::shrink(size_t reserve)
        {
            Buffer other;
            size_t cur_readableBytes = readableBytes();
            other.ensureWritableBytes(cur_readableBytes + reserve);
            other.append(peek(), cur_readableBytes);
            swap(other);
        }

        void Buffer::append(const char* data, size_t len) 
        {
            ensureWritableBytes(len);
            std::copy(data, data + len, beginWrite());
            hasWritten(len);
        }
        
        void Buffer::append(const std::string& str) 
        {
            append(str.data(), str.size());
        }

        void Buffer::append(const void* data, size_t len) 
        {
            append(static_cast<const char*>(data), len);
        }
        
        void Buffer::prepend(const void* data, size_t len) 
        {
            assert(len <= prependableBytes());
            readIndex_ -= len;
            const char* data_pre = static_cast<const char*>(data);
            std::copy(data_pre, data_pre + len, begin() + readIndex_);
        }
        
        // 结合栈上的空间，避免内存使用过大，提高内存使用率
        // 如果有1K个链接，每个连接都分配64K的（发生/接收）缓冲区的话，将独占640M内存
        // 而大多数时候，缓冲区的使用率都很低
        // https://www.cnblogs.com/solstice/archive/2011/04/17/2018801.html
        ssize_t Buffer::readFd(int fd, int* savedErrno) 
        {
            char extraBuf[65536];
            struct iovec vec[2];

            const size_t writable = writableBytes();
            vec[0].iov_base = beginWrite();
            vec[0].iov_len = writable;

            vec[1].iov_base = extraBuf;
            vec[1].iov_len = sizeof(extraBuf);

            std::cout << "before readFd()!!! WriteIndex at " << writeIndex_ << std::endl;

            // TODO : 为什么是这样比较？
            const int iovcnt = 2 ;// writableBytes() < sizeof(extraBuf) ? 2 : 1;
            const ssize_t n = sockets::readv(fd, vec, iovcnt);
            if(n < 0) 
            {
                *savedErrno = errno;
            } 
            else if(implicit_cast<size_t>(n) <= writable) 
            {
                writeIndex_ += n;
                std::cout << "readFd() over!!! WriteIndex at " << writeIndex_ << std::endl;

            } 
            else 
            {
                writeIndex_ = buffer_.size();
                append(extraBuf, n - writable);
            }
            return n;
        }
        
        void Buffer::makeSpace(size_t len) 
        {
            // 前面的空白加上也写不下
            if(writableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(writeIndex_ + len);
            }
            else
            {   
                // 前面有足够的空白
                assert(kCheapPrepend < readIndex_);
                size_t readable = readableBytes();
                std::copy(begin() + readIndex_,
                        begin() + writeIndex_,
                        begin() + kCheapPrepend);     
                readIndex_ = kCheapPrepend;
                writeIndex_ = readIndex_ + readable;  
                assert(readable == readableBytes());
            }
        }
    }
}