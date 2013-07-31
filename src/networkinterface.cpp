#include "networkinterface.hpp"

NetworkInterface::NetworkInterface(Kernel* kernel)
    : m_kernel(kernel)
{
}

std::pair<bool, LinkLayerAddress> NetworkInterface::neighborLinkLayerAddress(
        HostAddress address) const
{
    return std::pair<bool, LinkLayerAddress>(false, LinkLayerAddress());
}

void NetworkInterface::setNetworkAddress(const NetworkAddress &addr)
{
    m_networkAddress = addr;
}
