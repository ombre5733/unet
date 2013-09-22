#ifndef UNET_KERNEL_HPP
#define UNET_KERNEL_HPP

#include "config.hpp"

#include "bufferpool.hpp"
#include "event.hpp"
#include "networkheader.hpp"
#include "networkinterface.hpp"
#include "routingtable.hpp"

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

struct default_kernel_traits
{
    //! The size of one buffer in bytes.
    static const unsigned buffer_size = 256;

    //! The maximum number of buffers.
    static const unsigned max_num_buffers = 10;

    //! The maximum length of the kernel's event list.
    static const unsigned max_num_events = 20;

    //! The maximum number of interfaces which can be added to the kernel.
    static const unsigned max_num_interfaces = 5;

    //! The maximum number of neighbors, i.e. devices which can be accessed
    //! directly on a physical link without the need to route a message. This
    //! number can be smaller than the real number of neighbors. However,
    //! remember that sending to a neighbor which is not in the cache will
    //! create latency and traffic on the bus.
    static const unsigned max_num_cached_neighbors = 5;
};

//! The network kernel.
//! The network kernel is the central unit in the network implementation.
//! It keeps track of the interfaces and is in charge for routing messages.
template <typename TraitsT = default_kernel_traits>
class Kernel : public NetworkInterfaceListener
{
public:
    typedef TraitsT traits_t;

    //! Creates a network kernel.
    Kernel();

    //! Destroys the kernel.
    ~Kernel();

    //! Register an interface.
    //! Registers the interface \p ifc in the kernel.
    void addInterface(NetworkInterface* ifc);

    //! Sends a buffer.
    //! Sends the given \p message buffer to the neighbor specified by the
    //! \p destination address.
    void send(HostAddress destination, BufferBase* message);

    //! \reimp
    virtual BufferBase* allocate_buffer()
    {
        return m_bufferPool.allocate();
    }

    //! \reimp
    virtual void notify(Event event) {}

private:
    typedef BufferPool<traits_t::buffer_size,
                       traits_t::max_num_buffers> buffer_pool_t;
    //! The pool from which buffers are allocated.
    buffer_pool_t m_bufferPool;

    typedef EventList<traits_t::max_num_events> event_list_t;
    //! The list of events which has to be processed.
    event_list_t m_eventList;

    //! A thread to process the events.
    OperatingSystem::thread m_eventThread;

    //! The interfaces which have been registered in the kernel.
    NetworkInterface* m_interfaces[traits_t::max_num_interfaces];

    RoutingTable m_routingTable;

    void eventLoop();
    void handleMessageReceiveEvent(const Event& event);
    void handleMessageSendEvent(const Event& event);
};

template <typename TraitsT>
Kernel<TraitsT>::Kernel()
    : m_eventThread(&Kernel::eventLoop, this)
{
    for (unsigned idx = 0; idx < traits_t::max_num_interfaces; ++idx)
        m_interfaces[idx] = 0;
}

template <typename TraitsT>
Kernel<TraitsT>::~Kernel()
{
    m_eventList.enqueue(Event::createStopKernelEvent());
    m_eventThread.join();
}

template <typename TraitsT>
void Kernel<TraitsT>::addInterface(NetworkInterface *ifc)
{
    for (unsigned idx = 0; idx < traits_t::max_num_interfaces; ++idx)
        if (m_interfaces[idx] == 0)
        {
            m_interfaces[idx] = ifc;
            return;
        }

    ::uNet::throw_exception(-1);//! \todo system_error()
}

template <typename TraitsT>
void Kernel<TraitsT>::send(HostAddress destination, BufferBase* message)
{
    message->push_front(destination);
    m_eventList.enqueue(Event::createMessageSendEvent(message));
}

// ----=====================================================================----
//     Private methods
// ----=====================================================================----

template <typename TraitsT>
void Kernel<TraitsT>::eventLoop()
{
    bool stopEventThread = false;
    while (!stopEventThread)
    {
        Event event = m_eventList.retrieve();

        switch (event.type())
        {
            case Event::MessageReceive:
                handleMessageReceiveEvent(event);
                break;
            case Event::MessageSend:
                handleMessageSendEvent(event);
                break;
            case Event::StopKernel:
                stopEventThread = true;
                break;
            default:
                break;
        }
    }
}

template <typename TraitsT>
void Kernel<TraitsT>::handleMessageReceiveEvent(const Event& event)
{
    BufferBase* message = event.buffer();
    // Throw away malformed messages.
    if (message->size() < sizeof(NetworkHeader))
        return;
    UNET_ASSERT(event.networkInterface() != 0);

    const NetworkHeader* header
            = reinterpret_cast<const NetworkHeader*>(message->begin());

    HostAddress destAddr(header->destinationAddress);
    if (destAddr == event.networkInterface()->networkAddress().hostAddress()
        || destAddr.multicast())
    {
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
        //protocol_t::dispatch(header, message);
    }
    else
    {
        // The packet has to be routed.

    }
}

template <typename TraitsT>
void Kernel<TraitsT>::handleMessageSendEvent(const Event& event)
{
    BufferBase* message = event.buffer();
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
#endif
    HostAddress destination = message->pop_front<HostAddress>();

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table, which will map the destination address to
    // the one of the next neighbor.
    HostAddress routedDestination = m_routingTable.resolve(destination);

#if 0
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
#endif

    // We have not sent anything to this neighor, yet, or the neighbor has
    // been removed from the cache.
    // We have to loop over all interfaces and look for one which is in the
    // target's subnet.
    for (unsigned idx = 0; idx < traits_t::max_num_interfaces; ++idx)
    {
        NetworkInterface* ifc = m_interfaces[idx];
        if (!ifc)
            break;

        if (routedDestination.isInSubnet(ifc->networkAddress()))
        {
            /*
            nextHopInfo = m_nextHopCache.createNeighborCacheEntry(
                              routedDestination, ifc);
                              */

            // Add the network header and put the message in the neighbor's
            // queue.
            NetworkHeader header;
            header.destinationAddress = destination;
            header.sourceAddress = ifc->networkAddress().hostAddress();
            message->push_front(header);
            //sendToNeighbor(nextHopInfo, message);

            // Send out a neighbor solicitation.
            //sendNeighborSolicitation(ifc, routedDestination);

            // --- HACK
            ifc->send(LinkLayerAddress(), *message);
            // --- END OF HACK

            return;
        }
    }

    // Cannot find a route for this packet.
    // diagnostics.unknownRoute(destAddr);
    message->dispose();
}

} // namespace uNet

#endif // UNET_KERNEL_HPP
