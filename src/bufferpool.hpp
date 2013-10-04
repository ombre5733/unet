#ifndef UNET_BUFFERPOOL_HPP
#define UNET_BUFFERPOOL_HPP

#include "config.hpp"

#include "buffer.hpp"

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

//! A pool for buffers.
//! The BufferPool is an object pool for buffers. Both, the size of a buffer
//! and the number of buffers in the pool, are template parameters and thus
//! configurable at compile time. The parameter \p TBufferSize is the byte
//! size of each buffer and \p TNumBuffers is the total number of buffers
//! in the pool.
//!
//! The BufferPool implements the BufferDisposer interface. Every buffer
//! which is constructed via this pool holds a pointer back to the pool.
//! This means that every buffer can return itself to the pool from which
//! it was created.
template <unsigned TBufferSize, unsigned TNumBuffers>
class BufferPool : public BufferDisposer
{
public:
    //! The type of the buffer in this pool.
    typedef Buffer<TBufferSize, 4> buffer_type;

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
    //! If a buffer is available in the pool, it is allocated and a pointer
    //! to it is returned. If no buffer is available, a null-pointer is
    //! returned instead. The calling thread won't be blocked.
    buffer_type* try_allocate()
    {
        return m_pool.try_construct(this);
    }

protected:
    //! \reimp
    virtual void dispose(BufferBase* buffer)
    {
        release(static_cast<buffer_type*>(buffer));
    }

private:
    OperatingSystem::counting_object_pool<buffer_type, TNumBuffers> m_pool;
};

} // namespace uNet

#endif // UNET_BUFFERPOOL_HPP
