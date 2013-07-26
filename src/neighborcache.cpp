#include "neighborcache.hpp"

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
