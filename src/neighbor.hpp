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
//! A Neighbor object stores the available information about a neighbor. A
//! neighbor is a device which can be directly accessed on a physical link
//! without the need of routing a packet through a device.
//!
//! The Neighbor class combines the host address (the logical address) of a
//! device with its link-layer address (its physical address). The link-layer
//! address is important for sending a packet over a link as it is needed
//! by the interface to address the receiver.
//!
//! If neighbor discovery is in progress, no packets must be sent to a
//! neighbor. For this purpose, every neighbor keeps a list of pending packets
//! which have to be sent when the neighbor is known to be reachable. If
//! the reachability cannot be determined within a certain time, the packets
//! are dropped.
class Neighbor
{
public:
    //! An enumeration of discovery states.
    //!
    //! - Incomplete: The neighbor has just been created and the discovery
    //!   has not been completed, yet.
    //! - Reachable: The neighbor's reachability has been verified recently
    //!   and packets can simply be sent to it.
    //! - Stale: No response has been received from the neighbor for some
    //!   time and the reachability is questionable. Before sending a packet
    //!   to the neighbor, its reachability must be tested again.
    //! - Probe: The neighbor discovery is in progress. Further packets
    //!   addressed to this neighbor are delayed until after the neighbor
    //!   discovery.
    enum DiscoveryState
    {
        Incomplete,
        Reachable,
        Stale,
        Probe
    };

    Neighbor()
        : m_state(Incomplete),
          m_interface(0),
          m_neighborCacheHook(0)
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
        return m_delayedPackets;
    }

    //! Sets the network address.
    void setHostAddress(HostAddress address)
    {
        m_hostAddress = address;
    }

    //! Sets the network interface.
    //! Sets the interface via which this neighbor is reachable to \p ifc.
    void setInterface(NetworkInterface* ifc)
    {
        m_interface = ifc;
    }

    //! Sets the link-layer address.
    //! Sets the neighbor's link-layer address to \p addr.
    void setLinkLayerAddress(LinkLayerAddress addr)
    {
        m_linkLayerAddress = addr;
    }

    void setState(DiscoveryState state)
    {
        m_state = state;
    }

    DiscoveryState state() const
    {
        return m_state;
    }

private:
    //! The logical address used for addressing the neighbor from this device.
    HostAddress m_hostAddress;
    //! The discovery state of the neighbor.
    DiscoveryState m_state;
    //! The interface through which this neighbor can be accessed.
    NetworkInterface* m_interface;
    //! The link-layer address of the neighbor.
    LinkLayerAddress m_linkLayerAddress;
    //! A list of packets which have been delayed until after the neighbor
    //! discovery.
    BufferQueue m_delayedPackets;

public:
    //! The next neighbor in the cache.
    Neighbor* m_neighborCacheHook;
};

} // namespace uNet

#endif // UNET_NEIGHBOR_HPP
