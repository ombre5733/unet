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
          m_state(Incomplete)
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

    //! Returns the interface to the physical link.
    NetworkInterface* linkInterface() const
    {
        return m_interface;
    }

    //! Returns the link-layer address of the neighbor.
    LinkLayerAddress linkLayerAddress() const
    {
        return m_linkLayerAddress;
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
};

} // namespace uNet

#endif // UNET_NEIGHBOR_HPP
