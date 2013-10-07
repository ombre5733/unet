#include "networkaddress.hpp"

namespace uNet
{

bool HostAddress::isInSubnet(HostAddress subnetAddress,
                             std::uint16_t netmask) const
{
    return (m_address & netmask) == (subnetAddress.m_address & netmask);
}

bool HostAddress::isInSubnet(NetworkAddress subnetAddress) const
{
    std::uint16_t mask = subnetAddress.netmask();
    return m_address != 0
           && (m_address & mask)
              == (subnetAddress.hostAddress().address() & mask);
}

} // namespace uNet
