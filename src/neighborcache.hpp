#ifndef UNET_NEIGHBORCACHE_HPP
#define UNET_NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "neighbor.hpp"

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

//! The neighbor cache.
//! The NeighborCache keeps track of the accessed neighbors.
template <unsigned MaxNumNeighborsT>
class NeighborCache
{
public:
    //! Creates a neighbor cache.
    NeighborCache()
        : m_neighbors(0)
    {
    }

    Neighbor* createEntry(HostAddress address, NetworkInterface* ifc)
    {
        Neighbor* entry = m_neighborPool.construct();
        entry->setHostAddress(address);
        entry->setInterface(ifc);
        entry->m_neighborCacheHook = m_neighbors;
        m_neighbors = entry;
        return entry;
    }

    //! Performs a lookup in the cache.
    //! Searches the neighbor with the given logical \p address in the neighbor
    //! cache and returns a pointer to it. If no matching neighbor has been
    //! cached, a null-pointer is returned.
    Neighbor* find(HostAddress address) const
    {
        Neighbor* iter = m_neighbors;
        while (iter)
        {
            if (iter->address() == address)
                return iter;
            iter = iter->m_neighborCacheHook;
        }
        return 0;
    }

    // void update(HostAddress address);

private:
    //! The first neighbor in the list.
    Neighbor* m_neighbors;

    typedef OperatingSystem::object_pool<Neighbor, MaxNumNeighborsT>
        pool_t;
    //! The pool for the allocation of neighbors.
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
