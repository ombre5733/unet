#ifndef UNET_KERNEL_HPP
#define UNET_KERNEL_HPP

#include "config.hpp"

#include "bufferpool.hpp"
#include "networkinterface.hpp"

#include <weos/objectpool.hpp>
#include <weos/thread.hpp>

namespace uNet
{

class Event
{
    enum Type
    {
        LinkConnection,
        LinkConnectionLoss,
        MessageReceive,
        MessageSend
    };

private:
    Type m_type;
    Event* m_next;

    union
    {
        Buffer* m_buffer;
    } m_data;
};

struct default_kernel_traits
{
    static const int max_num_events = 20;
};

template <typename TraitsT = default_kernel_traits>
class Kernel
{
public:
    typedef TraitsT traits_t;

    typedef weos::counting_object_pool<
                Event, traits_t::max_num_events> event_pool_t;


    Kernel();

    void addInterface(NetworkInterface* ifc);
    event_pool_t& eventPool();

    void send(HostAddress destination, Buffer& message);

private:
    void eventLoop();

    //! A pool for events.
    event_pool_t m_eventPool;

    weos::thread m_eventThread;

    //! The interfaces which have been registered to the kernel.
    NetworkInterface* m_interfaces[10];
};

template <typename TraitsT>
Kernel<TraitsT>::Kernel()
{
}

template <typename TraitsT>
void Kernel<TraitsT>::addInterface(NetworkInterface *ifc)
{

}

template <typename TraitsT>
void Kernel<TraitsT>::send(HostAddress destination, Buffer& message)
{
#if 0
    // Perform a look-up in the destination cache.
    Neighbor* nextHopInfo = m_nextHopCache.lookupDestination(destination);
    if (nextHopInfo)
    {
        UnetHeader header;
        header.destinationAddress = destination;
        header.sourceAddress = nextHopInfo->interface()->networkAddress().hostAddress();
        message.push_front((uint8_t*)&header, sizeof(header));
        sendToNeighbor(nextHopInfo, message);
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
        message.push_front((uint8_t*)&header, sizeof(header));
        //m_nextHopCache.cacheDestination(destAddr, nextHopInfo);
        sendToNeighbor(nextHopInfo, message);
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
            message.push_front((uint8_t*)&header, sizeof(header));
            // Enqueue the packet in the neighbor info.
            sendToNeighbor(nextHopInfo, message);

            sendNeighborSolicitation(interface, routedDestination);
            return;
        }
    }

    // Cannot find a route for this packet.
    // diagnostics.unknownRoute(destAddr);
#endif
}

template <typename TraitsT>
void Kernel<TraitsT>::eventLoop()
{
    while (1)
    {

    }
}

} // namespace uNet

#endif // UNET_KERNEL_HPP
