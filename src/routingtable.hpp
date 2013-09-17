#ifndef UNET_ROUTINGTABLE_HPP
#define UNET_ROUTINGTABLE_HPP

#include "networkaddress.hpp"

namespace uNet
{

class RoutingTableEntry
{
public:
};

class RoutingTable
{
public:
    //! Resolves an address.
    //! Looks up the \p destination address in the routing table and returns
    //! the host address of the next neighbor, which is the next target for
    //! the message.
    HostAddress resolve(HostAddress destination) const;

private:
};

} // namespace uNet

#endif // UNET_ROUTINGTABLE_HPP
