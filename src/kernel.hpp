#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "buffer.hpp"
#include "mutex.hpp"
#include "networkinterface.hpp"
//#include "physicaladdresscache.hpp"
#include "routingtable.hpp"

// Network Control Protocol
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Type      |     Code      |           Checksum            |
// +---------------+---------------+---------------+---------------+

// Neighbor solicitation
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     Type      |     Code      |           Checksum            |
// +---------------+---------------+---------------+---------------+
// |        Target address         |                               |
// +---------------------------------------------------------------+
//
// NCP fields:
//     Type    1
//     Code    0

struct NetworkControlProtocolHeader
{
    NetworkControlProtocolHeader(uint8_t t, uint8_t c = 0)
        : type(t),
          code(c)
    {
    }

    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

struct NeighborSolicitation
{
    NeighborSolicitation()
        : header(1),
          reserved(0)
    {
    }

    NetworkControlProtocolHeader header;
    uint16_t targetAddress;
    uint16_t reserved;
};

struct NeighborAdvertisment
{

};

class NetworkControlProtocolHandler
{
public:
    void handle();

    void advertiseToNeighbors();
    void solicitNeighbor();
};


class NetworkHeaderTypeDispatcher
{
public:
    static void dispatch(uint8_t nextHeader)
    {
        if (nextHeader == 1)
        {
            std::cout << "received a Network Control Protocol msg" << std::endl;
        }
    }
};

struct Kernel_traits
{
    typedef std::vector<NetworkInterface*> network_interface_list_type;
};

class Kernel
{
public:
    Kernel();

    //void enqueuePacket(Buffer& packet);

    void addInterface(NetworkInterface* ifc);
    void addToPollingList(NetworkInterface& interface);

    void send(Buffer& packet);
    void receive(Buffer& packet);

private:
    mutex m_mutex;
    std::vector<NetworkInterface*> m_interfaces;
    RoutingTable m_routingTable;

    typedef boost::intrusive::member_hook<
            Buffer,
            Buffer::slist_hook_t,
            &Buffer::m_slistHook> list_options;
    typedef boost::intrusive::slist<
            Buffer,
            list_options,
            boost::intrusive::cache_last<true> > BufferList;
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
