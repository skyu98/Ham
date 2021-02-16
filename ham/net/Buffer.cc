#include "net/Buffer.h"
#include "net/Endian.h"
#include <assert.h>
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
        
        void Buffer::swap(const Buffer& rhs) 
        {
            
        }
        
        int8_t Buffer::peekInt8() const
        {
            
        }
        
        int16_t Buffer::peekInt16() const
        {
            
        }
        
        int32_t Buffer::peekInt32() const
        {
            
        }
        
        int32_t Buffer::peekInt64() const
        {
            
        }
        
        int8_t Buffer::readInt8() const
        {
            
        }
        
        int16_t Buffer::readInt16() const
        {
            
        }
        
        int32_t Buffer::readInt32() const
        {
            
        }
        
        int32_t Buffer::readInt64() const
        {
            
        }
        
        void Buffer::append(const char* data, size_t len) 
        {
            
        }
        
        void Buffer::append(const std::string& str) 
        {
            
        }
    }
}