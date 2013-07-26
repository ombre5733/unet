#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include "buffer.hpp"
#include "networkaddress.hpp"

class Kernel;

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

    explicit NetworkInterface(Kernel* kernel);

    //! Returns the domain to which the interface is connected.
    Domain domain() const;

    //! Returns the kernel to which the network interface belongs.
    Kernel* kernel() const
    {
        return m_kernel;
    }

    //! Returns the link-layer address of this interface.
    LinkLayerAddress linkLayerAddress() const;

    std::pair<bool, LinkLayerAddress> neighborLinkLayerAddress(
            NetworkAddress networkAddress) const;

    //! Returns the network address which has been set for this interface.
    NetworkAddress networkAddress() const
    {
        return m_networkAddress;
    }

    void setNetworkAddress(const NetworkAddress& addr);

    virtual void send(const LinkLayerAddress& address, Buffer& data) {}

private:
    //! The address which has been assigned to the interface.
    NetworkAddress m_networkAddress;
    Kernel* m_kernel;

    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        poll_list_hook_t;

    //! A hook for adding this interface to the polling list.
    poll_list_hook_t m_pollListHook;

    friend class Kernel;
};

#endif // NETWORKINTERFACE_HPP
