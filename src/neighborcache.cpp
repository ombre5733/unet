#include "neighborcache.hpp"

void Neighbor::setState(State state)
{
    m_state = state;
}

#if 0
void Neighbor::handleAdvertisment(
        LinkLayerAddress linkLayerAddress, bool solicited)
{
    bool override = false;

    if (solicited)
    {
        // The neighbor advertisment has been solicited.
        if (state == Incomplete)
        {
            // Don't care about the override flag as we do not have a
            // link-layer address, yet. Just record the link-layer address
            // and send the queued packets.
            setLinkLayerAddress(linkLayerAddress);
            setState(Reachable);
            process.sendQueue();
        }
        else if (override)
        {
            // The advertisment overrides our cached link-layer address.
            setLinkLayerAddress(linkLayerAddress);
            setState(Reachable);
        }
        else
        {
            // The advertisment does not override our cached address.
            if (linkLayerAddress == this->linkLayerAddress()())
                setState(Reachable);
            else
            {
                // The new address is different from the cached address.
                if (state == Reachable)
                    setState(Stale);
            }
        }
    }
    else
    {
        // The neighbor advertisment has not been solicited.
        if (state == Incomplete)
        {
            setLinkLayerAddress(linkLayerAddress);
            setState(Stale);
            process.sendQueue();
        }
        else if (override && linkLayerAddress != this->linkLayerAddress()())
        {
            // The advertisment overrides our cached link-layer address.
            setLinkLayerAddress(linkLayerAddress);
            setState(Stale);
        }
    }
}
#endif

Neighbor* NextHopCache::createNeighborCacheEntry(
        HostAddress address, NetworkInterface *interface)
{
    m_neighborCache.push_back(new Neighbor(address, interface));
    return m_neighborCache.back();
}

Neighbor* NextHopCache::lookupDestination(HostAddress address) const
{
    AddressToHopInfoMap::const_iterator iter = m_destinationCache.find(address);
    if (iter != m_destinationCache.end())
        return const_cast<Neighbor*>(iter->second);
    return 0;
}

Neighbor* NextHopCache::lookupNeighbor(HostAddress address) const
{
    for (NeighborCacheVector::const_iterator iter = m_neighborCache.begin(),
                                         end_iter = m_neighborCache.end();
         iter != end_iter; ++iter)
        if ((*iter)->address() == address)
            return *iter;
    return 0;
}




void TimeoutList::insert(Neighbor& neighbor, uint32_t timeout)
{
    if (neighbor.m_timeoutListHook.is_linked())
        erase(s_iterator_to(neighbor));

    neighbor.m_timeout = timeout;
    iterator iter = begin();
    while (iter != end() && iter->m_timeout <= timeout)
        ++iter;

    base_type::insert(iter, neighbor);
}
