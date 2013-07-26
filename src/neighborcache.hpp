#ifndef NEIGHBORCACHE_HPP
#define NEIGHBORCACHE_HPP

#include "networkaddress.hpp"

#include <map>

class NetworkInterface;


class AddressCache
{
public:
    struct InterfaceInfo
    {
    };

    const InterfaceInfo* lookup(HostAddress address) const
    {
        /*
        // 1. Have a look if we have already sent something to this address.
        if (m_destinationCache.contains(address))
            return m_destinationCache[address];
        // 2.
        */
        return 0;
    }

    std::map<HostAddress, const InterfaceInfo*> m_destinationCache;
    std::map<HostAddress, const InterfaceInfo*> m_neighborCache;
};


//! A cache for neighboring interfaces.
//!
//! The neighbor cache keeps track of the interface which are on-link and
//! thus directly reachable from this device. Data can be sent to a neighbor
//! interface without passing a router.
class NeighborCache
{
public:
    struct Entry
    {
        Entry() {}

        Entry(NetworkInterface* ifc, const LinkLayerAddress& lla)
            : interface(ifc),
              linkLayerAddress(lla)
        {
        }

        NetworkInterface* interface;
        LinkLayerAddress linkLayerAddress;
    };

    void cache(HostAddress address, NetworkInterface* interface,
               const LinkLayerAddress& linkLayerAddress);

    const Entry* lookup(HostAddress address) const;

private:
    typedef std::map<HostAddress, Entry> AddressToInterfaceInfoMap;
    AddressToInterfaceInfoMap m_addressToInterfaceInfo;
};

#endif // NEIGHBORCACHE_HPP
