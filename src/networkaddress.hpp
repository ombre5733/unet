#ifndef UNET_NETWORKADDRESS_HPP
#define UNET_NETWORKADDRESS_HPP

#include "config.hpp"

#include <boost/config.hpp>

#include <cstdint>

namespace uNet
{

class NetworkAddress;

struct all_device_multicast_t {};
struct link_local_all_device_multicast_t {};

BOOST_CONSTEXPR_OR_CONST all_device_multicast_t all_device_multicast = all_device_multicast_t();
BOOST_CONSTEXPR_OR_CONST link_local_all_device_multicast_t link_local_all_device_multicast = link_local_all_device_multicast_t();

//! Simply a host address.
//!
//! 16 bit total
//!
//! 0000 ... local
//! 0001 ... multicast
//! 0010 ... global
//!
//! link-local
//! site-local
//! global
//! special
//! - loopback
//! - unspecified
class HostAddress
{
public:
    //! Creates an unspecified host address.
    HostAddress()
        : m_address(0)
    {
    }

    HostAddress(std::uint16_t address)
        : m_address(address)
    {
    }

    //! Returns the address.
    std::uint16_t address() const
    {
        return m_address;
    }

    //! Checks if this is a multicast address.
    //! Returns \p true, if this host address is a multicast address.
    bool multicast() const
    {
        return (m_address & 0x8000) != 0;
    }

    //! Checks if this is an unspecified address.
    //! Returns \p true, if this host address is an unspecified address, i.e.
    //! if the address equals 0.
    bool unspecified() const
    {
        return m_address == 0;
    }

    //! Checks if this host address belongs to a sub-net.
    //! Returns \p true, if this host address belongs to the sub-net which
    //! is defined by \p subnetAddress (one address from the sub-net) and the
    //! associated \p netmask.
    bool isInSubnet(HostAddress subnetAddress, std::uint16_t netmask) const;

    bool isInSubnet(NetworkAddress subnetAddress) const;

    operator std::uint16_t() const
    {
        return m_address;
    }

    //! Returns the all-device multicast address.
    static HostAddress multicastAddress(all_device_multicast_t = all_device_multicast_t())
    {
        return 0x8000;
    }

    //! Returns the link-local all-device multicast address.
    static HostAddress multicastAddress(link_local_all_device_multicast_t)
    {
        return 0x8001;
    }

private:
    std::uint16_t m_address;
};

//! A complete network address consisting of the host address and the netmask.
class NetworkAddress
{
public:
    NetworkAddress()
        : m_netmask(0)
    {
    }

    NetworkAddress(std::uint16_t address, std::uint16_t netmask)
        : m_address(address),
          m_netmask(netmask)
    {
    }

    const HostAddress& hostAddress() const
    {
        return m_address;
    }

    std::uint16_t netmask() const
    {
        return m_netmask;
    }

private:
    HostAddress m_address;
    std::uint16_t m_netmask;
};

} // namespace uNet

#endif // UNET_NETWORKADDRESS_HPP
