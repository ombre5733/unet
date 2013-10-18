#include "routingtable.hpp"

namespace uNet
{

RoutingTable::RoutingTable()
    : m_numEntries(0)
{
}

void RoutingTable::addStaticRoute(NetworkAddress targetNetwork,
                                  HostAddress nextNeighbor)
{
    if (m_numEntries >= 10)
        ::uNet::throw_exception(-1); //! \todo system_error

    m_tableEntries[m_numEntries].m_targetNetwork = targetNetwork;
    m_tableEntries[m_numEntries].m_nextNeighbor = nextNeighbor;
    m_tableEntries[m_numEntries].m_static = true;
    ++m_numEntries;
}

HostAddress RoutingTable::resolve(HostAddress destination) const
{
    for (std::size_t idx = 0; idx < m_numEntries; ++idx)
    {
        if (destination.isInSubnet(m_tableEntries[idx].m_targetNetwork))
        {
            return m_tableEntries[idx].m_nextNeighbor;
        }
    }
    return destination;
}

} // namespace uNet
