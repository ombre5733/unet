#ifndef UNET_ROUTINGTABLE_HPP
#define UNET_ROUTINGTABLE_HPP

#include "networkaddress.hpp"

namespace uNet
{
//! An entry in the routing table.
class RoutingTableEntry
{
public:
    //! The address of the target network.
    NetworkAddress m_targetNetwork;
    //! The address of the next neighbor to which the packet has to be sent
    //! in order to reach the target network.
    HostAddress m_nextNeighbor;
    //! If set, this entry has been statically configured.
    bool m_static;
};

//! The routing table.
//! The RoutingTable resolves a destination address to a neighbor address. In
//! other words, for any in the network destination it returns the address to
//! which the message must be routed next.
class RoutingTable
{
public:
    RoutingTable();

    void addStaticRoute(NetworkAddress targetNetwork,
                        HostAddress nextNeighbor);

    //! Resolves an address.
    //! Looks up the \p destination address in the routing table and returns
    //! the host address of the next neighbor, which is the next target for
    //! the message.
    HostAddress resolve(HostAddress destination) const;

private:
    //! The table entries.
    RoutingTableEntry m_tableEntries[10];
    std::size_t m_numEntries;
};

} // namespace uNet

#endif // UNET_ROUTINGTABLE_HPP
