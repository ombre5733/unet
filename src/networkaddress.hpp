#ifndef NETWORKADDRESS_HPP
#define NETWORKADDRESS_HPP

#include <cstdint>

class NetworkAddress;

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

    HostAddress(uint16_t address)
        : m_address(address)
    {
    }

    //! Returns the address.
    uint16_t address() const
    {
        return m_address;
    }

    bool multicast() const
    {
        return (m_address & 0x1000) != 0;
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
    bool isInSubnet(const HostAddress& subnetAddress, uint16_t netmask) const;

    bool isInSubnet(const NetworkAddress& subnetAddress) const;

    operator uint16_t() const
    {
        return m_address;
    }

private:
    uint16_t m_address;
};

//! A complete network address consisting of the host address and the netmask.
class NetworkAddress
{
public:
    NetworkAddress()
        : m_netmask(0)
    {
    }

    NetworkAddress(uint16_t address, uint16_t netmask)
        : m_address(address),
          m_netmask(netmask)
    {
    }

    const HostAddress& hostAddress() const
    {
        return m_address;
    }

    uint16_t netmask() const
    {
        return m_netmask;
    }

private:
    HostAddress m_address;
    uint16_t m_netmask;
};

#endif // NETWORKADDRESS_HPP
