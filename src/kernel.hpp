#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "buffer.hpp"
#include "mutex.hpp"
#include "networkinterface.hpp"
//#include "physicaladdresscache.hpp"
#include "routingtable.hpp"


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
