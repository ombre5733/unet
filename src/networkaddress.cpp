#include "networkaddress.hpp"

bool HostAddress::isInSubnet(const HostAddress &subnetAddress,
                             uint16_t netmask) const
{
    return (m_address & netmask) == (subnetAddress.m_address & netmask);
}

bool HostAddress::isInSubnet(const NetworkAddress& subnetAddress) const
{
    return (m_address & subnetAddress.netmask())
            == (subnetAddress.hostAddress().address() & subnetAddress.netmask());
}
