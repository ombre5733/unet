#include "networkinterface.hpp"

NetworkInterface::NetworkInterface(Kernel* kernel)
    : m_kernel(kernel)
{
}

void NetworkInterface::setNetworkAddress(const NetworkAddress &addr)
{
    m_networkAddress = addr;
}
