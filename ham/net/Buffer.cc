#include "net/Buffer.h"
#include "net/Endian.h"
#include <assert.h>
#include <cstring>

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
    }
}