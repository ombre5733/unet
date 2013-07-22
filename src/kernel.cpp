#include "kernel.hpp"
#include "unetheader.hpp"

#include <cassert>


Kernel::Kernel()
{
}

void Kernel::addInterface(NetworkInterface *ifc)
{
    lock_guard<mutex> locker(m_mutex);
    m_interfaces.push_back(ifc);
}

void Kernel::addToPollingList(NetworkInterface& interface)
{
    lock_guard<mutex> locker(m_mutex);
    m_interfacesToPoll.push_back(interface);
}

void Kernel::send(Buffer& packet)
{
    lock_guard<mutex> locker(m_mutex);

    if (packet.size() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << "Sending packet with header: " << *header << std::endl;

    HostAddress destAddr(header->destinationAddress);
    // Perform a look-up in the destination cache.
    // ... todo ...

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table.
    destAddr = m_routingTable.resolve(destAddr);

    // Find an interface which we can use for sending this address.
    for (size_t idx = 0; idx < m_interfaces.size(); ++idx)
        if (destAddr.isInSubnet(m_interfaces[idx]->networkAddress()))
        {
            std::cout << "send via interface no " << idx << std::endl;
            m_interfaces[idx]->send(packet);
        }
}

void Kernel::receive(Buffer& packet)
{
    lock_guard<mutex> locker(m_mutex);

    if (packet.size() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << "Received packet with header: " << *header << std::endl;

    assert(packet.interface());

    HostAddress destAddr(header->destinationAddress);
    if (destAddr == packet.interface()->networkAddress().hostAddress())
    {
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
        std::cout << "this packet belongs to the link" << std::endl;
    }
    else
    {
        // The packet has to be routed.
        std::cout << "this packet has to be routed" << std::endl;
    }
}





class DestinationCache
{
public:
    // std::map<HostAddress/*destination*/, HostAddress>
};
