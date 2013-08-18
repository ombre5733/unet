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

    //! Checks if the pool is empty.
    //! Returns \p true if the pool is empty.
    bool empty() const
    {
        return m_pool.empty();
    }

    void release(buffer_type* const buffer)
    {
        std::cout << "BufferPool::release" << std::endl;
        m_pool.destroy(buffer);
    }

    buffer_type* try_acquire()
    {
        return acquire();
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

#endif // BUFFERPOOL_HPP
