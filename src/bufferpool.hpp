#ifndef BUFFERPOOL_HPP
#define BUFFERPOOL_HPP

#include "buffer2.hpp"
#include "staticobjectpool.hpp"

class Buffer2Pool
{
public:
    const BufferHandle& allocate()
    {
        BufferHandle* handle = m_bufferHandlePool.malloc();
        if (!handle)
            throw -1;

        Buffer2* buffer = m_bufferPool.malloc();
    }

private:
    static const int NUM_BUFFERS = 10;
    static const int NUM_BUFFER_HANDLES = 10;

    StaticObjectPool<Buffer2, NUM_BUFFERS> m_bufferPool;
    StaticObjectPool<BufferHandle, NUM_BUFFER_HANDLES> m_bufferHandlePool;
};

#endif // BUFFERPOOL_HPP
