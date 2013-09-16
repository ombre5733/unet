#ifndef UNET_BUFFERPOOL_HPP
#define UNET_BUFFERPOOL_HPP

#include "config.hpp"

#include "buffer.hpp"

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

template <unsigned TBufferSize, unsigned TNumBuffers>
class BufferPool : public BufferDisposer
{
public:
    typedef Buffer buffer_type; //! \todo Buffer2 should take the size as parameter

    //! Allocates a buffer from the pool.
    //! Allocates a buffer from the pool and returns a pointer to it. If the
    //! pool is empty, the calling thread is blocked until a buffer has been
    //! released.
    buffer_type* allocate()
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

    //! Tries to allocate a buffer.
    buffer_type* try_allocate()
    {
        //! \todo This needs to be implemented.
        return this->allocate();
    }

protected:
    //! \reimp
    virtual void dispose(buffer_type* buffer)
    {
        release(buffer);
    }

private:
    OperatingSystem::counting_object_pool<buffer_type, TNumBuffers> m_pool;
};

} // namespace uNet

#endif // UNET_BUFFERPOOL_HPP
