#ifndef UNET_NEXTHOPCACHE_HPP
#define UNET_NEXTHOPCACHE_HPP

namespace uNet
{

class NextHopCache
{
public:
    void createNeighborEntry(HostAddress address, NetworkInterface* ifc)
    {
        if (m_neighborCache.full())
        {
            Neighbor* neighbor = m_neighborCache.removeLast();
            m_routingTable.removeNeighbor(neighbor);
        }
        m_neighborCache.createEntry(address, ifc);
    }

    Neighbor* resolve(HostAddress destinationAddress)
    {

        HostAddress routedDestination
                = m_routingTable.resolve(destinationAddress);
        return m_neighborCache.find(routedDestination);
    }

private:
    NeighborCache m_neighborCache;
    RoutingTable m_routingTable;
};

} // namespace uNet

#endif // UNET_NEXTHOPCACHE_HPP
