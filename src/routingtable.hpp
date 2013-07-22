#ifndef ROUTINGTABLE_HPP
#define ROUTINGTABLE_HPP

#include "networkaddress.hpp"

class RoutingTable
{
public:
    HostAddress resolve(HostAddress destination) const;

private:
};

#endif // ROUTINGTABLE_HPP
