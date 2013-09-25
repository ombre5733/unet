#ifndef UNET_NETWORKPROTOCOL_HPP
#define UNET_NETWORKPROTOCOL_HPP

#include <cstdint>

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
    std::uint16_t sourceAddress;
    std::uint16_t destinationAddress;

    //! The maximum possible value for the hop count.
    static const std::uint8_t maxHopCount = 15;
};

} // namespace uNet

#endif // UNET_NETWORKPROTOCOL_HPP