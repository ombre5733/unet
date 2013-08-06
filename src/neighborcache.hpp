#ifndef NEIGHBORCACHE_HPP
#define NEIGHBORCACHE_HPP

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "networkaddress.hpp"

#include <boost/intrusive/list.hpp>

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
    NeighborCacheVector m_neighborCache; // TODO: use a boost::static_vector
};

#endif // NEIGHBORCACHE_HPP
