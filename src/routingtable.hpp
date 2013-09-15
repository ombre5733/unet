#ifndef UNET_ROUTINGTABLE_HPP
#define UNET_ROUTINGTABLE_HPP

#include "networkaddress.hpp"

namespace uNet
{

class RoutingTable
{
public:
    HostAddress resolve(HostAddress destination) const;

private:
};

} // namespace uNet

#endif // UNET_ROUTINGTABLE_HPP
