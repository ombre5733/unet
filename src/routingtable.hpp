#ifndef UNET_ROUTINGTABLE_HPP
#define UNET_ROUTINGTABLE_HPP

#include "networkaddress.hpp"

namespace uNet
{
class Neighbor;

class RoutingTableEntry
{
public:
    NetworkAddress m_address;
    Neighbor* m_neighbor;
    //! If set, this entry is static.
    bool m_static;
};

//! The routing table.
//! The RoutingTable resolves a destination address to a neighbor address. In
//! other words, for any in the network destination it returns the address to
//! which the message must be routed next.
class RoutingTable
{
public:
    //void addStaticRoute(HostAddress destination);

    //! Resolves an address.
    //! Looks up the \p destination address in the routing table and returns
    //! the host address of the next neighbor, which is the next target for
    //! the message.
    HostAddress resolve(HostAddress destination) const;

private:
    //! The table entries.
    RoutingTableEntry m_tableEntries[10];
};

} // namespace uNet

#endif // UNET_ROUTINGTABLE_HPP
