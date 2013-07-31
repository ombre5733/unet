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
    {
        lock_guard<mutex> locker(m_mutex);
        m_packetsToSend.push_back(packet);
    }
    senderThread();
}

namespace
{

void sendToNeighbor(NeighborInfo* neighbor, Buffer& packet)
{
    if (neighbor->state() == NeighborInfo::Reachable)
        neighbor->interface->send(neighbor->linkLayerAddress, packet);
    else
    {
        // TODO: If send queue is too long, remove the packet at
        // the front (the oldest one)
        //neighbor->sendQueue().push_back(packet);
    }
}

} // anonymous namespace

void Kernel::senderThread()
{
    lock_guard<mutex> locker(m_mutex);
    if (m_packetsToSend.empty())
        return;

    // Extract the first packet from the queue.
    Buffer& packet = m_packetsToSend.front();
    m_packetsToSend.pop_front();
    if (packet.size() < sizeof(UnetHeader))
    {
        // diagnostics.corruptHeader(packet.data());
        return;
    }
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << "Kernel - Trying to send packet with header: " << *header << std::endl;
    HostAddress destAddr(header->destinationAddress);

    // Perform a look-up in the destination cache.
    NeighborInfo* nextHopInfo = m_nextHopCache.lookupDestination(destAddr);
    if (nextHopInfo)
    {
        sendToNeighbor(nextHopInfo, packet);
        return;
    }

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table, which will map the destination address to
    // the one of the next neighbor.
    HostAddress routedDestAddr = m_routingTable.resolve(destAddr);
    nextHopInfo = m_nextHopCache.lookupNeighbor(routedDestAddr);
    if (nextHopInfo)
    {
        //m_nextHopCache.cacheDestination(destAddr, nextHopInfo);
        sendToNeighbor(nextHopInfo, packet);
        return;
    }

    // This neighbor has never been used before. We have to loop over all
    // interfaces and look for one which is in the target's subnet.
    for (size_t idx = 0; idx < m_interfaces.size(); ++idx)
    {
        NetworkInterface* interface = m_interfaces[idx];
        if (routedDestAddr.isInSubnet(interface->networkAddress()))
        {
            nextHopInfo = m_nextHopCache.createNeighborCacheEntry(
                              routedDestAddr, interface);
            sendToNeighbor(nextHopInfo, packet);

            std::cout << "Kernel - Send neighbor solicitation" << std::endl;
            Buffer* b = new Buffer;
            UnetHeader header;
            header.sourceAddress = interface->networkAddress().hostAddress();
            header.destinationAddress = HostAddress::multicastAddress();
            header.nextHeader = 1;
            b->push_back((uint8_t*)&header, sizeof(header));

            NeighborSolicitation solicitation;
            solicitation.targetAddress = destAddr;
            b->push_back((uint8_t*)&solicitation, sizeof(solicitation));

            std::pair<bool, LinkLayerAddress> lla
                    = interface->neighborLinkLayerAddress(routedDestAddr);
            if (lla.first)
            {
                std::cout << "Kernel - Send packet via interface no " << idx << std::endl;
                interface->send(lla.second, packet);
            }
            else
            {
                interface->broadcast(*b);

                // We can only send our packet, when we have received a
                // neighbor advertisment.
            }

            return;
        }
    }

    // Cannot find a route for this packet.
    // diagnostics.unknownRoute(destAddr);
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
