#include "networkinterface.hpp"

namespace uNet
{

// ----=====================================================================----
//     NetworkInterface
// ----=====================================================================----

NetworkInterface::NetworkInterface(NetworkInterfaceListener* listener)
    : m_listener(listener),
      m_name(0)
{
}

const char* NetworkInterface::name() const
{
    return m_name;
}

std::pair<bool, LinkLayerAddress> NetworkInterface::neighborLinkLayerAddress(
        HostAddress address) const
{
    return std::pair<bool, LinkLayerAddress>(false, LinkLayerAddress());
}

void NetworkInterface::setLinkLayerAddress(LinkLayerAddress address)
{
    m_linkLayerAddress = address;
}

void NetworkInterface::setName(const char *name)
{
    m_name = name;
}

void NetworkInterface::setNetworkAddress(const NetworkAddress &addr)
{
    m_networkAddress = addr;
}

void NetworkInterface::setListener(NetworkInterfaceListener* listener)
{
    m_listener = listener;
}

} // namespace uNet
