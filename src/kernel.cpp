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

namespace
{

void sendToNeighbor(Neighbor* neighbor, Buffer& packet)
{
    if (neighbor->state() == Neighbor::Reachable)
        neighbor->interface()->send(neighbor->linkLayerAddress(), packet);
    else
    {
        // TODO: If send queue is too long, remove the packet at
        // the front (the oldest one)
        neighbor->sendQueue().push_back(packet);
    }
}


void sendNeighborSolicitation(NetworkInterface* interface,
                              HostAddress destAddr)
{
    std::cout << "Kernel - Send neighbor solicitation" << std::endl;
    Buffer* b = BufferPool::allocate();

    NetworkControlProtocolMessageBuilder builder(*b);
    builder.createNeighborSolicitation(destAddr);
    //builder.addSourceLinkLayerAddressOption();

    UnetHeader header;
    header.sourceAddress = interface->networkAddress().hostAddress();
    header.destinationAddress = HostAddress::multicastAddress();
    header.nextHeader = 1;
    b->push_front((uint8_t*)&header, sizeof(header));

    std::pair<bool, LinkLayerAddress> lla
            = interface->neighborLinkLayerAddress(destAddr);
    if (lla.first)
    {
        std::cout << "Send unicast to neighor" << std::endl;
        interface->send(lla.second, *b);
    }
    else
    {
        std::cout << "Send a broadcast to neighbor" << std::endl;
        interface->broadcast(*b);
    }
}


} // anonymous namespace


void Kernel::send(NetworkInterface* sendingInterface, HostAddress destination,
                  Buffer& packet)
{

    // Perform a look-up in the destination cache.
    Neighbor* nextHopInfo = m_nextHopCache.lookupDestination(destination);
    if (nextHopInfo)
    {
        UnetHeader header;
        header.destinationAddress = destination;
        header.sourceAddress = nextHopInfo->interface()->networkAddress().hostAddress();
        packet.push_front((uint8_t*)&header, sizeof(header));
        sendToNeighbor(nextHopInfo, packet);
        return;
    }

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table, which will map the destination address to
    // the one of the next neighbor.
    HostAddress routedDestination = m_routingTable.resolve(destination);
    nextHopInfo = m_nextHopCache.lookupNeighbor(routedDestination);
    if (nextHopInfo)
    {
        UnetHeader header;
        header.destinationAddress = destination;
        header.sourceAddress = nextHopInfo->interface()->networkAddress().hostAddress();
        packet.push_front((uint8_t*)&header, sizeof(header));
        //m_nextHopCache.cacheDestination(destAddr, nextHopInfo);
        sendToNeighbor(nextHopInfo, packet);
        return;
    }

    // This neighbor has never been used before. We have to loop over all
    // interfaces and look for one which is in the target's subnet.
    for (size_t idx = 0; idx < m_interfaces.size(); ++idx)
    {
        NetworkInterface* interface = m_interfaces[idx];
        if (routedDestination.isInSubnet(interface->networkAddress()))
        {
            nextHopInfo = m_nextHopCache.createNeighborCacheEntry(
                              routedDestination, interface);

            UnetHeader header;
            header.destinationAddress = destination;
            header.sourceAddress = nextHopInfo->interface()->networkAddress().hostAddress();
            packet.push_front((uint8_t*)&header, sizeof(header));
            // Enqueue the packet in the neighbor info.
            sendToNeighbor(nextHopInfo, packet);

            sendNeighborSolicitation(interface, routedDestination);
            return;
        }
    }

    // Cannot find a route for this packet.
    // diagnostics.unknownRoute(destAddr);
#if 0
    {
        lock_guard<mutex> locker(m_mutex);
        m_packetsToSend.push_back(packet);
    }
    senderThread();
#endif
}

void Kernel::receive(Buffer& packet)
{
    lock_guard<mutex> locker(m_mutex);

    if (packet.dataSize() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.dataBegin());
    packet.advanceData(sizeof(header));
    std::cout << "Kernel - Received packet with header: " << *header << std::endl;

    assert(packet.interface());

    HostAddress destAddr(header->destinationAddress);
    if (destAddr == packet.interface()->networkAddress().hostAddress()
        || destAddr.multicast())
    {
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
        std::cout << "Kernel - this packet belongs to the link" << std::endl;
        protocol_t::dispatch(header, packet);
    }
    else
    {
        // The packet has to be routed.
        std::cout << "Kernel - this packet has to be routed" << std::endl;
    }
}
