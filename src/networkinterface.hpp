#ifndef UNET_NETWORKINTERFACE_HPP
#define UNET_NETWORKINTERFACE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "networkaddress.hpp"
#include "networkinterfacelistener.hpp"

namespace uNet
{

//! A network interface.
//! The network interface is the object which connects the network to the
//! outside world. The interface can either wrap a driver which itselves
//! connects to a physical data link (e.g. UART, SPI or CAN) or it can
//! wrap another protocol stack (e.g. USB or TCP/IP). The important aspect is
//! that the network kernel is able to send and receive data via a network
//! interface.
//!
//! A neighbor is another device which is attached to the same link as this
//! network interface. Such a device is also said to be on-link whereas devices
//! on other links are called off-link.
//!
//! A network interface can have a link-layer address in order to identify
//! this device on the link. As an example, if the network interface connects
//! to an I2C-bus, it needs to have an I2C-address such that it can
//! determine if a message on the bus is intended for this device. The format
//! of the link-layer address depends on the type of the network interface. If
//! the interface wraps a TCP/IP stack, the link-layer address could be a
//! combination of IP-address and TCP-port, for example. For point-to-point
//! connections such as UART, there is no need for a link-layer address as
//! the link consists of two end-points which can unambiguously send data to
//! each other. In this case, the link-layer address can be empty.
//!
//! The network kernel does not make use of the link-layer address directly.
//! However, if a message has to be sent to a neighbor, the sending network
//! interface needs the neighbor's link-layer address for routing the
//! message on the link. Therefore the kernel has to provide the link-layer
//! address to the sending interface even though it does not make use of
//! the link-layer address internally.
//!
//! The network interface can provide means to compute the link-layer address
//! of on-link devices right from the network address. This is advantageous
//! in at least two situations:
//! 1) If the link is a point-to-point connection between two devices (e.g.
//!    a UART link) and there is no need for a link-layer address.
//! 2) The network addresses on the link are configured statically and has been
//!    derived from the link-layer address.
//!
//! If the network interface cannot determine the link-layer address of a
//! neighbor, the kernel can resolve a network address to a link-layer address
//! by sending a (link-local) broadcast message to which the other device will
//! respond with its link-layer address. This procedure is more flexible as
//! the assginment of network addresses to link-layer addresses can be
//! dynamic. However, it certainly takes longer and is probably less reliable
//! due to the incurred communication.
class NetworkInterface
{
public:
    enum Domain
    {
        Private,
        Public
    };

    //! Creates a network interface.
    explicit NetworkInterface(NetworkInterfaceListener* listener);

    //! Sends a broadcast.
    //! Broadcasts the \p packet to all interfaces on the link.
    //! \note After sending the \p packet, the buffer has to be disposed.
    //! \note This function must be thread-safe.
    virtual void broadcast(BufferBase& packet) = 0;

    //! Returns the domain to which the interface is connected.
    Domain domain() const;

    //! Checks if the link to which this interface connects has addresses.
    //! Returns \p true, if the link to which this interface connects has
    //! addresses.
    virtual bool linkHasAddresses() const = 0;

    //! Returns the link-layer address.
    //! Returns the link-layer address which uniquely identifies this interface
    //! on the physical link. If the link does not have addresses, the method
    //! should return an unspecified link-layer address.
    LinkLayerAddress linkLayerAddress() const
    {
        return m_linkLayerAddress;
    }

    //! Returns the listener.
    //! Returns the listener which has been attached to this network interface.
    NetworkInterfaceListener* listener() const
    {
        return m_listener;
    }

    //! Returns the interface's name.
    //! Returns the interface's name. By default, this is an empty string.
    const char* name() const;

    //! \todo Remove this function.
    virtual std::pair<bool, LinkLayerAddress> neighborLinkLayerAddress(
            HostAddress address) const;

    //! Returns the network address.
    //! Returns the logical address which has been set for this interface.
    NetworkAddress networkAddress() const
    {
        return m_networkAddress;
    }

    //! Sets the link-layer address.
    //! Sets the link-layer address of the interface to \p address.
    void setLinkLayerAddress(LinkLayerAddress address);

    //! Sets a name.
    //! Assigns the \p name to this interface. This is useful for debugging.
    void setName(const char* name);

    //! Sets the logical address.
    //! Sets the logical address of the link to \p addr.
    void setNetworkAddress(const NetworkAddress& addr);

    //! Sends a packet.
    //! Sends the \p packet to another interface on the same link which
    //! is identified through its link-layer \p address. If the link does
    //! not have link-layer addresses (because it is a point-to-point
    //! connection such as e.g. a UART), the \p address should be ignored.
    //! \note After sending the \p packet, the buffer has to be disposed.
    //! \note This function must be thread-safe.
    virtual void send(const LinkLayerAddress& address, BufferBase& packet) = 0;

    //! Sets a listener.
    //! Attaches the given \p listener to this network interface.
    void setListener(NetworkInterfaceListener* listener);

private:
    //! The link-layer address of this interface.
    LinkLayerAddress m_linkLayerAddress;
    //! The listener which is notified about events from this interface.
    NetworkInterfaceListener* m_listener;
    //! The name of the interface.
    const char* m_name;
    //! The address which has been assigned to the interface.
    NetworkAddress m_networkAddress;
};

} // namespace uNet

#endif // UNET_NETWORKINTERFACE_HPP
