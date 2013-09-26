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
    //! Allocates a buffer and returns a pointer to it.
    virtual BufferBase* allocateBuffer() = 0;

    //! Notifies the listener.
    //! Notifies the listener about an \p event. This method is called by
    //! the NetworkInterface to which this listener is attached.
    virtual void notify(const Event& event) = 0;
};

} // namespace uNet

#endif // UNET_NETWORKINTERFACELISTENER_HPP
