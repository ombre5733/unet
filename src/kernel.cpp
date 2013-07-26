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
    std::cout << "Kernel - Trying to send packet with header: " << *header << std::endl;

    HostAddress destAddr(header->destinationAddress);
    // Perform a look-up in the destination cache.
    // ... todo ...

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table.
    destAddr = m_routingTable.resolve(destAddr);

    const NeighborCache::Entry* cacheEntry
            = m_neighborCache.lookup(destAddr);
    if (cacheEntry)
    {
        cacheEntry->interface->send(cacheEntry->linkLayerAddress, packet);
        return;
    }

    // Find an interface which we can use for sending this address.
    for (size_t idx = 0; idx < m_interfaces.size(); ++idx)
        if (destAddr.multicast()
            || destAddr.isInSubnet(m_interfaces[idx]->networkAddress()))
        {
            std::cout << "Kernel - Send packet via interface no " << idx << std::endl;
            m_interfaces[idx]->send(LinkLayerAddress(), packet);
        }
}

void Kernel::receive(Buffer& packet)
{
    lock_guard<mutex> locker(m_mutex);

    if (packet.size() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << "Kernel - Received packet with header: " << *header << std::endl;

    assert(packet.interface());

    HostAddress destAddr(header->destinationAddress);
    if (destAddr == packet.interface()->networkAddress().hostAddress()
        || destAddr.multicast())
    {
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
        std::cout << "Kernel - this packet belongs to the link" << std::endl;
        NetworkHeaderTypeDispatcher::dispatch(header->nextHeader);
    }
    else
    {
        // The packet has to be routed.
        std::cout << "Kernel - this packet has to be routed" << std::endl;
    }
}





class DestinationCache
{
public:
    // std::map<HostAddress/*destination*/, HostAddress>
};
