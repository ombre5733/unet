#ifndef UNET_NETWORKINTERFACELISTENER_HPP
#define UNET_NETWORKINTERFACELISTENER_HPP

#include "event.hpp"

namespace uNet
{
class BufferBase;

//! A listener for a network interface.
//! The NetworkInterfaceListener is attached to a NetworkInterface and will
//! receive notifications from it.
class NetworkInterfaceListener
{
public:
    //! Allocates a buffer.
    //! Allocates a buffer and returns a pointer to it. If no buffer is
    //! available, the caller is blocked until a buffer is returned.
    virtual BufferBase* allocateBuffer() = 0;

    /*
    //! Tries to allocate a buffer.
    //! Tries to allocate a buffer. If one was available, a pointer to it
    //! is returned. Otherwise, a null-pointer is returned. The caller
    //! will never be blocked.
    virtual BufferBase* tryAllocateBuffer() = 0;

    //! Tries to allocate a buffer.
    //! Tries to allocate a buffer within the given \p timeout and returns
    //! a pointer to it. If no buffer is available before the duration expires,
    //! a null-pointer is returned.
    virtual BufferBase* tryAllocateBufferFor(
        const chrono::milliseconds& timeout) = 0;
    */

    //! Notifies the listener.
    //! Notifies the listener about an \p event. This method is called by
    //! the NetworkInterface to which this listener is attached.
    virtual void notify(const Event& event) = 0;
};

} // namespace uNet

#endif // UNET_NETWORKINTERFACELISTENER_HPP
