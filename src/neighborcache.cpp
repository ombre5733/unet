#include "neighborcache.hpp"

void NeighborInfo::setState(State state)
{
    m_state = state;
}


NeighborInfo* NextHopCache::createNeighborCacheEntry(
        HostAddress address, NetworkInterface *interface)
{
    m_neighborCache.push_back(NeighborInfo(address, interface));
    return &m_neighborCache.back();
}

NeighborInfo* NextHopCache::lookupDestination(HostAddress address) const
{
    AddressToHopInfoMap::const_iterator iter = m_destinationCache.find(address);
    if (iter != m_destinationCache.end())
        return const_cast<NeighborInfo*>(iter->second);
    return 0;
}

NeighborInfo* NextHopCache::lookupNeighbor(HostAddress address) const
{
    for (NeighborCacheVector::const_iterator iter = m_neighborCache.begin(),
                                         end_iter = m_neighborCache.end();
         iter != end_iter; ++iter)
        if (iter->address() == address)
            return const_cast<NeighborInfo*>(&*iter);
    return 0;
}


#if 0
void NeighborCache::cache(HostAddress address, NetworkInterface *interface,
                          const LinkLayerAddress &linkLayerAddress)
{
    m_addressToInterfaceInfo[address] = Entry(interface, linkLayerAddress);
}

const NeighborInfo* NextHopCache::lookup(HostAddress address) const
{
    AddressToHopInfoMap::const_iterator iter
            = m_neighborCache.find(address);
    if (iter != m_neighborCache.end())
        return iter->second;
    else
        return 0;
}

NeighborInfo *NextHopCache::lookupNeighbor(HostAddress address) const
{
    AddressToHopInfoMap::const_iterator iter
            = m_neighborCache.find(address);
    if (iter != m_neighborCache.end())
        return iter->second;
    else
        return 0;
}
#endif







void NeighborCache::cache(HostAddress address, NetworkInterface *interface,
                          const LinkLayerAddress &linkLayerAddress)
{
    m_addressToInterfaceInfo[address] = Entry(interface, linkLayerAddress);
}

const NeighborCache::Entry* NeighborCache::lookup(HostAddress address) const
{
    AddressToInterfaceInfoMap::const_iterator iter
            = m_addressToInterfaceInfo.find(address);
    if (iter != m_addressToInterfaceInfo.end())
        return &iter->second;
    else
        return 0;
}
