#ifndef NEIGHBORCACHE_HPP
#define NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "networkaddress.hpp"
#include "staticobjectpool.hpp"

#include <boost/intrusive/list.hpp>

//#include <boost/container/static_vector.hpp>

#include <list>
#include <map>
#include <vector>

const int MAX_NUM_NEIGHBORS = 10;

class NetworkInterface;

//! A neighbor on a physical link.
//! A Neighbor is a device which can be accessed on a physical link.
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

    //! The link-layer address of the neighbor.
    LinkLayerAddress linkLayerAddress() const
    {
        return m_linkLayerAddress;
    }

    BufferQueue& sendQueue()
    {
        return m_sendQueue;
    }

    void setState(State state);

    State state() const
    {
        return m_state;
    }

private:
    //! The logical address used for accessing the neighbor from this device.
    HostAddress m_hostAddress;
    //! The interface through which this neighbor can be accessed.
    NetworkInterface* m_interface;
    //! The link-layer address of the neighbor.
    LinkLayerAddress m_linkLayerAddress;
    //! A list of packets which have been queued for sending.
    BufferQueue m_sendQueue;
    State m_state;
    uint8_t m_numTimeouts;

    uint32_t m_timeout;

public:
    typedef boost::intrusive::list_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        timeout_list_hook_t;
    timeout_list_hook_t m_timeoutListHook;

    friend class TimeoutList;
};

//! A linked-list of neighbor timeouts.
//! The TimoutList is a linked-list of neighbors in which the neighbors are
//! sorted by their timeout.
class TimeoutList : public boost::intrusive::list<
                               Neighbor,
                               boost::intrusive::member_hook<
                                   Neighbor,
                                   Neighbor::timeout_list_hook_t,
                                   &Neighbor::m_timeoutListHook> >
{
    typedef boost::intrusive::list<
                Neighbor,
                boost::intrusive::member_hook<
                    Neighbor,
                    Neighbor::timeout_list_hook_t,
                    &Neighbor::m_timeoutListHook> > base_type;

public:
    void insert(Neighbor& neighbor, uint32_t timeout);
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
    NeighborCacheVector m_neighborCache; // TODO: use a StaticObjectPool

    StaticObjectPool<Neighbor, MAX_NUM_NEIGHBORS> m_neighborPool;
};

#endif // NEIGHBORCACHE_HPP
