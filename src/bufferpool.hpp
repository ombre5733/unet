#ifndef BUFFERPOOL_HPP
#define BUFFERPOOL_HPP

#include "buffer2.hpp"
#include "staticobjectpool.hpp"


template <unsigned TBufferSize>
class BufferPool : public BufferDisposer
{
public:
    typedef Buffer2 buffer_type; //! \todo Buffer2 should take the size as parameter

    buffer_type* acquire()
    {
        std::cout << "BufferPool::acquire" << std::endl;
        return m_pool.construct(this);
    }

    void release(buffer_type* const buffer)
    {
        std::cout << "BufferPool::release" << std::endl;
        m_pool.destroy(buffer);
    }

protected:
    virtual void operator() (buffer_type* buffer)
    {
        release(buffer);
    }

private:
    static const int NUM_BUFFERS = 10;
    StaticObjectPool<buffer_type, NUM_BUFFERS> m_pool;
};


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
