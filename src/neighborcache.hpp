#ifndef NEIGHBORCACHE_HPP
#define NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "networkaddress.hpp"

//#include <boost/container/static_vector.hpp>

#include <list>
#include <map>
#include <vector>

class NetworkInterface;

//! A collection of neighbor data.
class Neighbor
{
public:
    //! An enumeration of states of the neighbor.
    enum State
    {
        Incomplete,
        Reachable,
        Stale,
        Delay,
        Probe
    };

    Neighbor()
        : m_state(Incomplete),
          m_numTimeouts(0)
    {
    }

    Neighbor(HostAddress address, NetworkInterface* ifc)
        : m_hostAddress(address),
          m_interface(ifc),
          m_state(Incomplete)
    {
    }

    HostAddress address() const
    {
        return m_hostAddress;
    }

    NetworkInterface* interface() const
    {
        return m_interface;
    }

    LinkLayerAddress linkLayerAddress() const
    {
        return m_linkLayerAddress;
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

private:
    HostAddress m_hostAddress;
    NetworkInterface* m_interface;
    LinkLayerAddress m_linkLayerAddress;
    State m_state;
    BufferList m_sendQueue;
    uint8_t m_numTimeouts;
};

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
    NeighborCacheVector m_neighborCache; // TODO: use a boost::static_vector
};

#endif // NEIGHBORCACHE_HPP
