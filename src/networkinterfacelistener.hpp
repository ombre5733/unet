#ifndef UNET_NETWORKINTERFACELISTENER_HPP
#define UNET_NETWORKINTERFACELISTENER_HPP

#include "event.hpp"

namespace uNet
{
//! A listener for a network interface.
//! The NetworkInterfaceListener is attached to a NetworkInterface and will
//! receive notifications from it.
class NetworkInterfaceListener
{
public:
    //! Notifies the listener.
    //! Notifies the listener about an \p event. This method is called by
    //! the NetworkInterface to which this listener is attached.
    virtual void notify(Event event) = 0;
};

} // namespace uNet

#endif // UNET_NETWORKINTERFACELISTENER_HPP
