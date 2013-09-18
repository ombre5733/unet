#ifndef UNET_NEIGHBORCACHE_HPP
#define UNET_NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "neighbor.hpp"

namespace uNet
{

//! The neighbor cache.
//! The NeighborCache keeps track of all neighbors of a device, i.e. it keeps
//! a list of all devices which can be accessed without the need for routing
//! a message.
template <unsigned MaxNumNeighborsT>
class NeighborCache
{
public:
    //Neighbor* find(HostAddress address) const;
    // void update(HostAddress address);

private:
    Neighbor* m_neighbors;

    typedef OperatingSystem::object_pool<Neighbor, MaxNumNeighborsT, null_mutex>
        pool_t;
    pool_t m_neighborPool;
};

#if 0
// - lookupDestination() Called from the kernel if it wants to send a
//   packet to a destination.
class NextHopCache
{
public:
    Neighbor* lookupDestination(HostAddress address) const;
    Neighbor* lookupNeighbor(HostAddress address) const;
    Neighbor* createNeighborCacheEntry(HostAddress address,
                                       NetworkInterface* interface);
    void removeNeighborCacheEntry();

private:
    typedef std::map<HostAddress, const Neighbor*> AddressToHopInfoMap;
    typedef std::list<Neighbor*> NeighborCacheVector;

    AddressToHopInfoMap m_destinationCache;  // TODO: use a sorted list
    NeighborCacheVector m_neighborCache; // TODO: use a StaticObjectPool

    StaticObjectPool<Neighbor, MAX_NUM_NEIGHBORS> m_neighborPool;
};
#endif

} // namespace uNet

#endif // UNET_NEIGHBORCACHE_HPP
