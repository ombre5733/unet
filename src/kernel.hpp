#ifndef UNET_KERNEL_HPP
#define UNET_KERNEL_HPP

#include "config.hpp"

#include "bufferpool.hpp"
#include "event.hpp"
#include "networkcontrolprotocol.hpp"
#include "networkprotocol.hpp"
#include "networkinterface.hpp"
#include "routingtable.hpp"
#include "protocol/protocolhandlerchain.hpp"

#include <OperatingSystem/OperatingSystem.h>

#include <cstddef>

#include "neighborcache.hpp"
namespace uNet
{
//! The default kernel traits.
//! These traits define the behaviour of the kernel when no other traits
//! are provided by the user.
struct default_kernel_traits
{
    //! The size of one buffer in bytes.
    static const unsigned buffer_size = 256;

    //! The maximum number of buffers. If this value is set to zero, the number
    //! of buffers is unlimited and only restricted by the available memory.
    static const unsigned max_num_buffers = 10;

    //! The maximum length of the kernel's event list. If this value is set
    //! to zero, no limit is imposed on the number of events.
    static const unsigned max_num_events = 20;

    //! The maximum number of interfaces which can be added to the kernel.
    static const unsigned max_num_interfaces = 5;

    //! The maximum number of neighbors, i.e. devices which can be accessed
    //! directly on a physical link without the need to route a message. This
    //! number can be smaller than the real number of neighbors. However,
    //! remember that sending to a neighbor which is not in the cache will
    //! create latency and traffic on the bus.
    static const unsigned max_num_cached_neighbors = 5;

    //! A list of protocols which are attached to the kernel.
    typedef boost::mpl::vector<> protocol_list_t;
};

namespace detail
{
template <unsigned TBufferSize, unsigned TMaxNumBuffers, bool TGreaterZero>
struct buffer_pool_type_dispatch_helper;

template <unsigned TBufferSize, unsigned TMaxNumBuffers>
struct buffer_pool_type_dispatch_helper<TBufferSize, TMaxNumBuffers, true>
{
    typedef BufferPool<TBufferSize, TMaxNumBuffers> type;
};

// A helper struct to dispatch the type of the buffer pool for the kernel.
template <unsigned TBufferSize, unsigned TMaxNumBuffers>
struct buffer_pool_type_dispatcher
{
    typedef typename buffer_pool_type_dispatch_helper<
                         TBufferSize, TMaxNumBuffers,
                         (TMaxNumBuffers > 0)>::type type;
};

template <unsigned TMaxNumEvents, bool TGreaterZero>
struct event_list_type_dispatch_helper;

template <unsigned TMaxNumEvents>
struct event_list_type_dispatch_helper<TMaxNumEvents, true>
{
    typedef EventList<TMaxNumEvents> type;
};

template <unsigned TMaxNumEvents>
struct event_list_type_dispatcher
{
    typedef typename event_list_type_dispatch_helper<
                         TMaxNumEvents, (TMaxNumEvents > 0)>::type type;
};

} // namespace detail

//! The network kernel.
//! The network kernel is the central unit in the network implementation.
//! It keeps track of the interfaces and is in charge for routing packets.
template <typename TraitsT = default_kernel_traits>
class Kernel : public NetworkInterfaceListener,
               public NcpHandler<Kernel<TraitsT> >
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

    template <typename TProtocol>
    TProtocol* protocolHandler()
    {
        return m_protocolChain.template get<TProtocol>();
    }

    //! Sends a packet.
    //! Sends the given \p packet to the neighbor specified by the
    //! \p destination address. Before sending, a network header is prepended
    //! to the packet. The type of the first header in the packet is encoded
    //! in the \p headerType. This type will be incorporated in the network
    //! header.
    void send(HostAddress destination, std::uint8_t headerType, BufferBase* packet);

    //! \reimp
    virtual BufferBase* allocateBuffer()
    {
        return m_bufferPool.allocate();
    }

    virtual BufferBase* tryAllocateBuffer()
    {
        return m_bufferPool.try_allocate();
    }

    //! \reimp
    virtual void notify(const Event& event)
    {
        m_eventList.enqueue(event);
    }


    //! \internal
    void send(NetworkInterface* ifc, LinkLayerAddress linkLayerAddress, BufferBase& packet);

private:
    //! The type of the buffer pool.
    typedef typename detail::buffer_pool_type_dispatcher<
                         traits_t::buffer_size,
                         traits_t::max_num_buffers>::type buffer_pool_t;
    //! The pool from which buffers are allocated.
    buffer_pool_t m_bufferPool;

    //! The type of the event list.
    typedef typename detail::event_list_type_dispatcher<
                         traits_t::max_num_events>::type event_list_t;
    //! The list of events which has to be processed.
    event_list_t m_eventList;

    //! A thread to process the events.
    OperatingSystem::thread m_eventThread;

    //! The interfaces which have been registered in the kernel.
    NetworkInterface* m_interfaces[traits_t::max_num_interfaces];

    RoutingTable m_routingTable;

    //! The type of the protocol chain.
    typedef typename make_protocol_handler_chain<
                         typename traits_t::protocol_list_t>::type
                         protocol_chain_t;
    //! The protocol chain.
    protocol_chain_t m_protocolChain;

    void eventLoop();
    void handlePacketReceiveEvent(const Event& event);
    void handlePacketSendEvent(const Event& event);

    void handleSendLinkLocalBroadcastEvent(const Event& event);

    void sendNeighborSolicitation(NetworkInterface* ifc, HostAddress destAddr);

public:
    //! \todo This needs to be combined with a destination cache and the routing table.
    NeighborCache<traits_t::max_num_cached_neighbors> nc;
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
void Kernel<TraitsT>::send(HostAddress destination, std::uint8_t headerType,
                           BufferBase* packet)
{
    // Be strict on what we send.
    if (destination.unspecified())
        ::uNet::throw_exception(-1);//! \todo Use a system_error

    NetworkProtocolHeader header;
    header.destinationAddress = destination;
    header.nextHeader = headerType;
    header.length = packet->size() + sizeof(NetworkProtocolHeader);
    packet->push_front(header);
    m_eventList.enqueue(Event::createMessageSendEvent(packet));
}

template <typename TraitsT>
void Kernel<TraitsT>::send(NetworkInterface* ifc, LinkLayerAddress linkLayerAddress, BufferBase& packet)
{
    if (packet.size() < sizeof(NetworkProtocolHeader))
        ::uNet::throw_exception(-1);

    HostAddress destinationAddress
            = detail::getNetworkProtocolDestinationAddress(packet.begin());

    if (destinationAddress.unspecified())
        ::uNet::throw_exception(-1);//! \todo Use a system_error

    detail::setNetworkProtocolSourceAddress(
                packet.begin(), ifc->networkAddress().hostAddress());

    if (destinationAddress.multicast()
        || (linkLayerAddress.unspecified() && ifc->linkHasAddresses()))
    {
        ifc->broadcast(packet);
    }
    else
    {
        ifc->send(linkLayerAddress, packet);
    }
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
                handlePacketReceiveEvent(event);
                break;
            case Event::MessageSend:
                handlePacketSendEvent(event);
                break;
            case Event::StopKernel:
                stopEventThread = true;
                break;

            case Event::SendLinkLocalBroadcast:
                handleSendLinkLocalBroadcastEvent(event);
                break;
            default:
                break;
        }
    }
}

template <typename TraitsT>
void Kernel<TraitsT>::handlePacketReceiveEvent(const Event& event)
{
    UNET_ASSERT(event.networkInterface() != 0);

    BufferBase* packet = event.buffer();

    if (packet->size() < sizeof(NetworkProtocolHeader))
    {
        packet->dispose();
        return;
    }
    ProtocolMetaData metaData;
    metaData.npHeader = packet->copy_front<NetworkProtocolHeader>();
    metaData.networkInterface = event.networkInterface();

    // Throw away malformed packets.
    if (   metaData.npHeader.version != 1
        || metaData.npHeader.length != packet->size()
        || metaData.npHeader.destinationAddress.unspecified()
        || metaData.npHeader.sourceAddress.multicast())
    {
        packet->dispose();
        return;
    }

    if (   metaData.npHeader.destinationAddress
               == metaData.networkInterface->networkAddress().hostAddress()
        || metaData.npHeader.destinationAddress.multicast())
    {
        packet->moveBegin(sizeof(NetworkProtocolHeader));
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
        if (metaData.npHeader.nextHeader == 1)
        {
            // This is an NCP message.
            NcpHandler<Kernel<TraitsT> >::receive(metaData, *packet);
        }
        else
        {
            m_protocolChain.dispatch(metaData, *packet);
        }
    }
    else
    {
        // The packet has to be routed.

        // Do not route packets which have an unspecified source address
        // because we cannot send a reply.
        if (   metaData.npHeader.sourceAddress.unspecified()
            || metaData.npHeader.hopCount == 0)
        {
            packet->dispose();
            return;
        }

        //! \todo Implementation missing.
        packet->dispose();
    }
}

template <typename TraitsT>
void Kernel<TraitsT>::handleSendLinkLocalBroadcastEvent(const Event& event)
{
    std::cout << "handleSendLinkLocalBroadcastEvent" << std::endl;
    BufferBase* packet = event.buffer();
    UNET_ASSERT(packet->size() >= sizeof(NetworkProtocolHeader));
    UNET_ASSERT(event.networkInterface());
    event.networkInterface()->broadcast(*packet);
}

template <typename TraitsT>
void Kernel<TraitsT>::handlePacketSendEvent(const Event& event)
{
    BufferBase* packet = event.buffer();
    NetworkProtocolHeader header = packet->copy_front<NetworkProtocolHeader>();
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

    // We have not found an entry in the destination cache. The next step is to
    // consult the routing table, which will map the destination address to
    // the one of the next neighbor.
    HostAddress routedDestination = m_routingTable.resolve(
                                        header.destinationAddress);

    // Look up the neighbor in the cache.
    Neighbor* cachedNeighbor = nc.find(routedDestination);
    if (cachedNeighbor)
    {
        switch (cachedNeighbor->state())
        {
            case Neighbor::Incomplete:
            case Neighbor::Probe:
                cachedNeighbor->sendQueue().push_back(*packet);
                break;
            case Neighbor::Reachable:
                cachedNeighbor->networkInterface()->send(
                            cachedNeighbor->linkLayerAddress(), *packet);
                break;
            case Neighbor::Stale:
                // We are not completely sure if the neighbor is reachable.
                // We transmit the packet.
                assert(0);
        }

        return;
    }

    // We have not sent anything to this neighor, yet, or the neighbor has
    // been removed from the cache. Now the problem is that we do not know the
    // link-layer address of the target. We loop over all interfaces and
    // look for one which is in the target's subnet, send a Neighbor
    // Solicitation over this interface and queue the packet until we receive
    // a neighbor advertisment.
    for (unsigned idx = 0; idx < traits_t::max_num_interfaces; ++idx)
    {
        NetworkInterface* ifc = m_interfaces[idx];
        if (!ifc)
            break;
        if (!routedDestination.isInSubnet(ifc->networkAddress()))
            continue;

        cachedNeighbor = nc.createEntry(routedDestination, ifc);
        /*
            nextHopInfo = m_nextHopCache.createNeighborCacheEntry(
                              routedDestination, ifc);
        */

        // Put the packet in the neighbor's queue. It will be sent when we get
        // a Neighbor Advertisment.
        cachedNeighbor->sendQueue().push_back(*packet);

        // Send out a neighbor solicitation.
        sendNeighborSolicitation(ifc, routedDestination);
        return;
    }

    // Cannot find a route for this packet.
    // diagnostics.unknownRoute(destAddr);
    packet->dispose();
}

template <typename TraitsT>
void Kernel<TraitsT>::sendNeighborSolicitation(NetworkInterface* ifc,
                                               HostAddress destAddr)
{
    //! \todo This is a blocking allocation. When all buffers are taken,
    //! we run into a deadlock here. A solution might be to keep a list
    //! of pending solicitations in the kernel and periodically try to
    //! send them.
    //! Another possibility is to have a internal buffer pool soley for the
    //! purpose of sending neighbor solicitations.
    BufferBase* buffer = allocateBuffer();

    NetworkControlProtocolMessageBuilder builder(*buffer);
    builder.createNeighborSolicitation(destAddr);
    if (ifc->linkHasAddresses())
    {
        builder.addSourceLinkLayerAddressOption(ifc->linkLayerAddress());
    }

    NetworkProtocolHeader header;
    header.sourceAddress = ifc->networkAddress().hostAddress();
    header.destinationAddress = HostAddress::multicastAddress(
                                    link_local_all_device_multicast);
    header.nextHeader = 1;
    header.length = buffer->size() + sizeof(NetworkProtocolHeader);
    buffer->push_front(header);

    std::pair<bool, LinkLayerAddress> lla
            = ifc->neighborLinkLayerAddress(destAddr);
    if (lla.first)
    {
        ifc->send(lla.second, *buffer);
    }
    else
    {
        ifc->broadcast(*buffer);
    }
}

} // namespace uNet

#endif // UNET_KERNEL_HPP
