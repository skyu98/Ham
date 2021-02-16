#include "net/Buffer.h"
#include "net/Endian.h"
#include <assert.h>
#include <cstring>
#include <algorithm>

namespace ham
{
    namespace net
    {
        const char Buffer::kCRLF[]= "\r\n";

        Buffer::Buffer()
            : buffer_(kInitialSize + kCheapPrepend),
              readIndex(kCheapPrepend), 
              writeIndex(kCheapPrepend)
        {
            assert(readableBytes() == 0);
            assert(writableBytes() == kInitialSize);
            assert(prependableBytes() == kCheapPrepend);
        }
        
        void Buffer::swap(Buffer& rhs) 
        {
            std::swap(buffer_, rhs.buffer_);
            std::swap(readIndex, rhs.readIndex);
            std::swap(writeIndex, rhs.writeIndex);
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
        
        int32_t Buffer::readInt64() 
        {
            int64_t res = peekInt64();
            retrieveInt64();
            return res;
        }
        
        void Buffer::retrieveAll() 
        {
            // 全部读完，read和write都回到起点
            readIndex = kCheapPrepend; 
            writeIndex = kCheapPrepend; 
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
        
        void Buffer::makeSpace(size_t len) 
        {
            // 前面的空白加上也写不下
            if(writableBytes() + prependableBytes() < len + kCheapPrepend)
            {
                buffer_.resize(writeIndex + len);
            }
            else
            {   
                // 前面有足够的空白
                assert(kCheapPrepend < readIndex);
                size_t readable = readableBytes();
                std::copy(begin() + readIndex,
                        begin() + writeIndex,
                        begin() + kCheapPrepend);     
                readIndex = kCheapPrepend;
                writeIndex = readIndex + readable;  
                assert(readable == readableBytes());
            }
        }
    }
}