#ifndef UNET_NETWORKPROTOCOL_HPP
#define UNET_NETWORKPROTOCOL_HPP

#include "networkaddress.hpp"

#include <cstdint>
#include <cstring>

namespace uNet
{

/*!
Network Protocol

The network protocol carries enough information to route data from one
interface in the network to another. It has the following header:

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|Version|HopCnt |  Next header  |            Length             |
+-------+-------+---------------+---------------+---------------+
|        Source address         |      Destination address      |
+---------------+---------------+---------------+---------------+
\endcode

The \p Version is the protocol version. The \p HopCnt field is decremented
whenever the message is routed. If the \p HopCnt is zero, the message is
not routed any longer but is dropped by the kernel.
The <tt>Next header</tt> encodes the type of the header in the payload.
The \p Length is the total length of the message including this header. The
<tt>Source address</tt> and <tt>Destination address</tt> fields contain
the source and destination addresses of the sending and receiving
interfaces.

Network Address

A network address consists of two parts: (i) a prefix which is common to
all devices on a link and (ii) a unique device address.

TODO: Describe
100  --> 1 11
110  --> 11 1

Unspecified Address

The address 0 is defined as the unspecified address. The unspecified address
can be used as source address by interfaces to which no address has been
assigned, yet. This is required to allow dynamic address assignment over the
network protocol, for example.

Multicast Addresses

Multicast Addresses are used to simultaneously send data to more than one
device. The following multicast addresses are defined:
- Link-local all-device multicast address: Sends data to all devices on one
  link. These messages must not be routed.
- All-device multicast address: Sends data to all devices. These messages
  are routed.

A network message must be discarded if any of the following is true.
- The \p Length field does not match the data size.
- The destination address is unspecified.
- The source address is a multicast address.

A network message must not be routed if the source address is unspecified.

*/

struct NetworkProtocolHeader
{
    NetworkProtocolHeader()
        : version(1),
          hopCount(maxHopCount)
    {
    }

    std::uint8_t version : 4;
    std::uint8_t hopCount : 4;
    std::uint8_t nextHeader;
    std::uint16_t length;
    HostAddress sourceAddress;
    HostAddress destinationAddress;

    //! The maximum possible value for the hop count.
    static const std::uint8_t maxHopCount = 15;
};

namespace detail
{
inline
HostAddress getNetworkProtocolDestinationAddress(std::uint8_t* buffer)
{
    HostAddress destinationAddress;
    std::memcpy(&destinationAddress,
                buffer + offsetof(NetworkProtocolHeader, destinationAddress),
                sizeof(NetworkProtocolHeader::destinationAddress));
    return destinationAddress;
}

inline
void setNetworkProtocolSourceAddress(std::uint8_t* buffer, HostAddress addr)
{
    std::memcpy(buffer + offsetof(NetworkProtocolHeader, sourceAddress),
                &addr,
                sizeof(NetworkProtocolHeader::sourceAddress));
}

} // namespace detail

} // namespace uNet

#endif // UNET_NETWORKPROTOCOL_HPP
