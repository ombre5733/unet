#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "buffer.hpp"
#include "mutex.hpp"
#include "neighborcache.hpp"
#include "networkinterface.hpp"
//#include "physicaladdresscache.hpp"
#include "routingtable.hpp"

#include "unetheader.hpp"
#include "networkcontrolprotocol.hpp"

#include <map>


template <typename DerivedT>
class NetworkHeaderTypeDispatcher
{
public:
    void dispatch(uint8_t nextHeader)
    {
        std::cout << "NetworkHeaderTypeDispatcher - nextHeader = " << (uint16_t)nextHeader << std::endl;
        if (nextHeader == 1)
        {
            derived()->onNetworkControlProtocol();
        }
    }

    void onNetworkControlProtocol()
    {
        std::cout << "received a Network Control Protocol msg" << std::endl;
    }

private:
    DerivedT* derived()
    {
        return static_cast<DerivedT*>(this);
    }
};

struct Kernel_traits
{
    typedef std::vector<NetworkInterface*> network_interface_list_type;
};

class PV
{
public:
    virtual void swapSourceAndDestination() = 0;
};

template <typename DerivedT>
class Protocol : public NetworkControlProtocol<DerivedT>
{
public:
    void dispatch(const UnetHeader* header, Buffer &packet)
    {
        switch (header->nextHeader)
        {
            case NetworkControlProtocol<DerivedT>::headerType:
                NetworkControlProtocol<DerivedT>::dispatch(header, packet);
                break;

            default:
                // ... dignostics ...
                break;
        }
    }
};

class Kernel : public Protocol<Kernel>
{
public:
    typedef Protocol<Kernel> protocol_t;

    Kernel();

    //void enqueuePacket(Buffer& packet);

    void addInterface(NetworkInterface* ifc);
    void addToPollingList(NetworkInterface& interface);

    //! Sends a packet.
    //! Sends the packet \p packet via the \p sendingInterface to the
    //! requested \p destination.
    void send(NetworkInterface* sendingInterface, HostAddress destination,
              Buffer& packet);
    void receive(Buffer& packet);

    void senderThread();

    void onNcpNeighborAdvertisment(Buffer &packet)
    {
        std::cout << "Kernel - Received NCP neighbor advertisment" << std::endl;
    }

    void onNcpNeighborSolicitation(const NeighborSolicitation* solicitation,
                                   Buffer& packet)
    {
        std::cout << "Kernel - Received NCP neighbor solicitation" << std::endl;

        NeighborSolicitation sol = *solicitation;
        NcpOption::SourceLinkLayerAddress* srcLlaOption
                = NcpOption::find<NcpOption::SourceLinkLayerAddress>(
                      packet.dataBegin() + sizeof(NeighborSolicitation),
                      packet.dataEnd());
        if (srcLlaOption)
        {
            std::cout << "Kernel - Have src LLA" << std::endl;
        }

        packet.clearData();

        std::cout << "Kernel - Send neighbor advertisment" << std::endl;

        NetworkControlProtocolMessageBuilder builder(packet);
        builder.createNeighborAdvertisment(sol.targetAddress);
        builder.addTargetLinkLayerAddressOption(
                    packet.interface()->linkLayerAddress());

        //! \todo Whould be nicer if we could swap the source and destination
        //! in-place.
        //packet.swap
        {
            UnetHeader* hdr = reinterpret_cast<UnetHeader*>(packet.begin());
            std::cout << "Changing from " << hdr->sourceAddress << "-" << hdr->destinationAddress;
            hdr->destinationAddress = hdr->sourceAddress;
            hdr->sourceAddress = packet.interface()->networkAddress().hostAddress();
            std::cout << "   to   " << hdr->sourceAddress << "-" << hdr->destinationAddress << std::endl;
        }

        //! \todo What is the correct interface and address?
        packet.interface()->broadcast(packet);
    }

private:
    mutex m_mutex;
    std::vector<NetworkInterface*> m_interfaces; // TODO: use a static_vector
    RoutingTable m_routingTable;
    NextHopCache m_nextHopCache;
    BufferList m_packetsToSend;

    typedef boost::intrusive::member_hook<
            NetworkInterface,
            NetworkInterface::poll_list_hook_t,
            &NetworkInterface::m_pollListHook> network_interface_options;
    typedef boost::intrusive::slist<
            NetworkInterface,
            network_interface_options,
            boost::intrusive::cache_last<true> > NetworkInterfacePollList;
    NetworkInterfacePollList m_interfacesToPoll;
};

#endif // KERNEL_HPP
