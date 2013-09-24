#ifndef UNET_NEIGHBOR_HPP
#define UNET_NEIGHBOR_HPP

#include "../config.hpp"

#include "buffer.hpp"
#include "linklayeraddress.hpp"
#include "networkaddress.hpp"

namespace uNet
{
class NetworkInterface;

//! A neighbor on a physical link.
//! A Neighbor object stores the available information about a neighbor, which
//! is a device which can be directly accessed on a physical link without
//! the need of routing a message through a device.
//!
//! The Neighbor class combines the host address (the logical address) of a
//! device with its link-layer address (its physical address). This information
//! is important for sending a message over a physical link as the link has
//! to address the receiver with its link-layer address rather than its
//! logical address.
//!
//! If neighbor discovery is in progress, no messages must be sent to a
//! neighbor. For this purpose, every neighbor keeps a list of pending messages
//! which have to be sent when the neighbor is known to be reachable. If
//! the reachability cannot be determined within a certain time, the messages
//! must be dropped.
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
        : m_interface(0),
          m_state(Incomplete),
          m_neighborCacheHook(0)
    {
    }

    Neighbor(HostAddress address, NetworkInterface* ifc)
        : m_hostAddress(address),
          m_interface(ifc),
          m_state(Incomplete)
    {
    }

    //! Returns the host address.
    HostAddress address() const
    {
        return m_hostAddress;
    }

    //! Returns the link-layer address of the neighbor.
    LinkLayerAddress linkLayerAddress() const
    {
        return m_linkLayerAddress;
    }

    //! Returns the interface to the physical link.
    NetworkInterface* networkInterface() const
    {
        return m_interface;
    }

    //! Returns the buffer queue.
    BufferQueue& sendQueue()
    {
        return m_sendQueue;
    }

    void setState(State state)
    {
        m_state = state;
    }

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
    //! A list of messages which have been queued for sending.
    BufferQueue m_sendQueue;


    State m_state;

public:
    //! The next neighbor in the cache.
    Neighbor* m_neighborCacheHook;
};

} // namespace uNet

#endif // UNET_NEIGHBOR_HPP
