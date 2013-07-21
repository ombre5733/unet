#include "networkinterface.hpp"

NetworkInterface::NetworkInterface()
{
}

void NetworkInterface::setNetworkAddress(const NetworkAddress &addr)
{
    m_networkAddress = addr;
}
