#ifndef NEIGHBORCACHE_HPP
#define NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "networkaddress.hpp"

//#include <boost/container/static_vector.hpp>

#include <list>
#include <map>
#include <vector>

class NetworkInterface;

//! A collection of neighbor data.
class NeighborInfo
{
    typedef boost::intrusive::member_hook<
            Buffer,
            Buffer::slist_hook_t,
            &Buffer::m_slistHook> list_options;
    typedef boost::intrusive::slist<
            Buffer,
            list_options,
            boost::intrusive::cache_last<true> > BufferList;

public:
    enum State
    {
        Incomplete,
        Reachable,
        Stale,
        Delay,
        Probe
    };

    NeighborInfo()
        : m_state(Incomplete)
    {
    }

    NeighborInfo(HostAddress address, NetworkInterface* ifc)
        : m_hostAddress(address),
          interface(ifc),
          m_state(Incomplete)
    {
    }

    HostAddress address() const
    {
        return m_hostAddress;
    }

    BufferList& sendQueue()
    {
        return m_sendQueue;
    }

    void setState(State state);

    State state() const
    {
        return m_state;
    }

    NetworkInterface* interface;
    LinkLayerAddress linkLayerAddress;

private:
    HostAddress m_hostAddress;
    State m_state;
    BufferList m_sendQueue;
};

// - lookupDestination() Called from the kernel if it wants to send a
//   packet to a destination.
class NextHopCache
{
public:
    NeighborInfo* lookupDestination(HostAddress address) const;
    NeighborInfo* lookupNeighbor(HostAddress address) const;
    NeighborInfo* createNeighborCacheEntry(HostAddress address,
                                           NetworkInterface* interface);
    void removeNeighborCacheEntry();

private:
    typedef std::map<HostAddress, const NeighborInfo*> AddressToHopInfoMap;
    typedef std::list<NeighborInfo> NeighborCacheVector;

    AddressToHopInfoMap m_destinationCache;  // TODO: use a sorted list
    NeighborCacheVector m_neighborCache; // TODO: use a boost::static_vector
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
