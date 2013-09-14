#ifndef UNET_BUFFERPOOL_HPP
#define UNET_BUFFERPOOL_HPP

#include "config.hpp"

#include "buffer2.hpp"

#include <weos/objectpool.hpp>

namespace uNet
{

template <unsigned TBufferSize, unsigned TNumBuffers>
class BufferPool : public BufferDisposer
{
public:
    typedef Buffer2 buffer_type; //! \todo Buffer2 should take the size as parameter

    //! Acquires a buffer from the pool.
    buffer_type* acquire()
    {
        return m_pool.construct(this);
    }

    //! Checks if the pool is empty.
    //! Returns \p true if the pool is empty.
    bool empty() const
    {
        return m_pool.empty();
    }

    //! Releases a buffer.
    //! Releases the \p buffer which must have been acquired from this pool.
    void release(buffer_type* const buffer)
    {
        m_pool.destroy(buffer);
    }

    buffer_type* try_acquire()
    {
        return acquire();
    }

protected:
    //! \reimp
    virtual void dispose(buffer_type* buffer)
    {
        release(buffer);
    }

private:
    weos::counting_object_pool<buffer_type, TNumBuffers> m_pool;
};

} // namespace uNet

#endif // UNET_BUFFERPOOL_HPP
