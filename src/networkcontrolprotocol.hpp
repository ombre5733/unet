#ifndef UNET_NETWORKCONTROLPROTOCOL_HPP
#define UNET_NETWORKCONTROLPROTOCOL_HPP

// Network Control Protocol
// http://tools.ietf.org/html/rfc4861

/*!
Network Control Protocol

The network control protocol (NCP) is used to perform basic network management
such as making a node known to other participants on the same bus or to query
the network address if only a link-layer address is known.

All messages of the network control protocol (NCP) start with the following
header:

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+
\endcode

The \p Type field encodes the NCP message type. The \p Code field can be
used for sub-typing and is set to zero if the \p Type does not have
sub-types.

\todo Describe the computation of the checksum

Neighbor solicitation message

A neighbor solicitation is sent whenever a device needs to verify the
reachability of another device on the bus or to resolve its link-layer address.

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+
|        Target address         |           reserved            |
+---------------+---------------+---------------+---------------+
| Options ...
+---------------
\endcode

Network protocol fields:
    Source address
            The address of the sending interface or the unspecified address
            if no address has been assigned to the interface, yet.

    Destination address
            The multicast address or the target address.

    HopCnt  The maximum possible hop count.

NCP fields:
    Type    1
    Code    0

The <tt>Target address</tt> is the address of the device which is solicited.

Only the <tt>Source link-layer address</tt> is allowed as an option in the
neighbor solicitation message.

Note: The neighbor solicitation message can be sent from an unspecified source
address. This is used to probe if a wanted address on a link is still available
or already taken by another device. If the address is in use, the targeted
device sends the corresponding neighbor advertisment as a link-local broadcast.


Neighbor advertisment message

A neighbor advertisment is sent by a device to make itself known to the
other devices on the bus. A neighbor advertisment can either be sent
spontaneously or as a response to a neighbor solicitation.

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+
|        Target address         |S|         reserved            |
+---------------+---------------+---------------+---------------+
| Options ...
+---------------
\endcode

Network protocol fields:
    Source address
            The address of the interface via which the advertisment is sent.

    Destination address
            The source address of the soliciation message or, if the
            solicitation's source address is the unspecified address, the
            multicast address.

    HopCnt  The maximum possible hop count.

NCP fields:
    Type    2
    Code    0

The \p S (solicited) bit is set, if the neighbor advertisment is sent as a
response to a neighbor solicitation. It is kept clear, if the advertisment
has been sent spontaneously (e.g. after a connection to a link has been
established).

The <tt>Target address</tt> is the same address as in the corresponding
neighbor solicitation. If the advertisment is sent unsolicited, it is the
address of the interface which changed its link-layer address.


NCP options

NCP options can be appended to an NCP message. Every NCP option starts with
the following header
\code
0                   1
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------
|   NcpOptType  |     Length    | ...
+---------------+---------------+-------
\endcode

The \p NcpOptType field contains the type of the option and \p Length its
length (tbd: in units of bytes or 32-bit words?). An implementation must
silently ignore options which it does not understand. The \p Length field
allows to skip these options.

The following options are available:

Source link-layer address option

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   NcpOptType  |     Length    |             ????              |
+---------------+---------------+---------------+---------------+
| ????????????????????????????????????????????????????????????? |
+---------------+---------------+---------------+---------------+
\endcode

NCP option fields
    NcpOptType
            1

    Length  ???

    Link-layer address
            The link-layer address of the source which sent the Neighbor
            Solicitation.


Target link-layer address option

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   NcpOptType  |     Length    |             ????              |
+---------------+---------------+---------------+---------------+
| ????????????????????????????????????????????????????????????? |
+---------------+---------------+---------------+---------------+
\endcode

NCP option fields
    NcpOptType
            1

    Length  ???

    Link-layer address
            The link-layer address of the target which sent the Neighbor
            Advertisment.

*/

#include "config.hpp"

#include "buffer.hpp"
#include "event.hpp"
#include "neighborcache.hpp"
#include "networkaddress.hpp"
#include "networkinterface.hpp"
#include "networkprotocol.hpp"
#include "linklayeraddress.hpp"

#include "protocol/protocol.hpp"

#include <cstring>
#include <cstdint>

namespace uNet
{

struct NetworkControlProtocolHeader
{
    NetworkControlProtocolHeader(std::uint8_t t = 0, std::uint8_t c = 0)
        : type(t),
          code(c)
    {
    }

    std::uint8_t type;
    std::uint8_t code;
    std::uint16_t checksum;
};

struct NeighborSolicitation
{
    static const int ncpType = 1;

    NeighborSolicitation()
        : header(1),
          reserved(0)
    {
    }

    NetworkControlProtocolHeader header;
    HostAddress targetAddress;
    std::uint16_t reserved;
};

struct NeighborAdvertisment
{
    static const int ncpType = 2;

    NeighborAdvertisment()
        : header(2),
          solicited(false),
          reserved(0)
    {
    }

    NetworkControlProtocolHeader header;
    HostAddress targetAddress;
    bool solicited : 1;
    std::uint16_t reserved : 15;
};

namespace NcpOption
{

struct NetworkControlProtocolOption
{
    static const int unitByteSize = 4;

    NetworkControlProtocolOption(std::uint8_t _type, std::uint8_t byteSize)
        : type(_type),
          length((byteSize + unitByteSize - 1) / unitByteSize)
    {
    }

    std::uint16_t byteSize() const
    {
        return length * unitByteSize;
    }

    // The type of the NCP option.
    std::uint8_t type;
    // The length of the NCP option in units of 4 bytes.
    std::uint8_t length;
};

struct SourceLinkLayerAddress
{
    static const int ncpOptionType = 1;

    SourceLinkLayerAddress(LinkLayerAddress lla)
        : ncpOptionHeader(ncpOptionType, sizeof(*this))
    {
        std::memcpy(m_linkLayerAddress, &lla, sizeof(LinkLayerAddress));
    }

    LinkLayerAddress linkLayerAddress() const
    {
        LinkLayerAddress lla;
        std::memcpy(&lla, m_linkLayerAddress, sizeof(LinkLayerAddress));
        return lla;
    }

    NetworkControlProtocolOption ncpOptionHeader;
    std::uint8_t m_linkLayerAddress[sizeof(LinkLayerAddress)];
};

struct TargetLinkLayerAddress
{
    static const int ncpOptionType = 2;

    TargetLinkLayerAddress(LinkLayerAddress lla)
        : ncpOptionHeader(ncpOptionType, sizeof(*this))
    {
        std::memcpy(m_linkLayerAddress, &lla, sizeof(LinkLayerAddress));
    }

    LinkLayerAddress linkLayerAddress() const
    {
        LinkLayerAddress lla;
        std::memcpy(&lla, m_linkLayerAddress, sizeof(LinkLayerAddress));
        return lla;
    }

    NetworkControlProtocolOption ncpOptionHeader;
    std::uint8_t m_linkLayerAddress[sizeof(LinkLayerAddress)];
};

//! Searches an NCP option inside a range.
//! This function takes a range of data delimited by a \p begin and \p end
//! iterator and searches for an NCP option whose type is specified by the
//! template parameter \p OptT. If the data contains such an option, a pointer
//! to it is returned. Otherwise, the function returns a null-pointer.
template <typename OptT>
OptT* find(std::uint8_t* begin, std::uint8_t* end)
{
    while (static_cast<std::size_t>(end - begin)
           >= sizeof(NetworkControlProtocolOption))
    {
        const NetworkControlProtocolOption* optHeader
                = reinterpret_cast<const NetworkControlProtocolOption*>(begin);
        if (optHeader->type == OptT::ncpOptionType)
            return reinterpret_cast<OptT*>(begin);
        begin += optHeader->byteSize();
    }
    return 0;
}

} // namespace NcpOption

//! A helper class to create network control protocol messages.
class NetworkControlProtocolMessageBuilder
{
public:
    //! Creates a builder for NCP messages.
    explicit NetworkControlProtocolMessageBuilder(BufferBase& buffer)
        : m_buffer(buffer)
    {
    }

    //! Creates a neighbor advertisment message.
    void createNeighborAdvertisment(HostAddress targetAddress,
                                    bool solicited = false)
    {
        NeighborAdvertisment advertisment;
        advertisment.targetAddress = targetAddress;
        advertisment.solicited = solicited;
        m_buffer.push_back(advertisment);
    }

    //! Creates a neighbor solicitation message.
    void createNeighborSolicitation(HostAddress targetAddress)
    {
        NeighborSolicitation solicitation;
        solicitation.targetAddress = targetAddress;
        m_buffer.push_back(solicitation);
    }

    //! Adds a source link-layer address option.
    void addSourceLinkLayerAddressOption(LinkLayerAddress linkLayerAddress)
    {
        NcpOption::SourceLinkLayerAddress llaOpt(linkLayerAddress);
        m_buffer.push_back(llaOpt);
    }

    //! Adds a target link-layer address option.
    void addTargetLinkLayerAddressOption(LinkLayerAddress linkLayerAddress)
    {
        NcpOption::TargetLinkLayerAddress llaOpt(linkLayerAddress);
        m_buffer.push_back(llaOpt);
    }

private:
    //! The buffer in which the message will be built.
    BufferBase& m_buffer;
};

template <typename KernelT>
class NcpHandler
{
public:
    //! Handles a network control protocol message.
    //! Handles an incoming NCP message with its associated network protocol
    //! \p metaData. The NCP payload is passed in the \p packet buffer.
    void receive(const ProtocolMetaData& metaData, BufferBase& packet)
    {
        // Perform some sanity checks.
        // - The packet is large enough
        // - The packet must not have been routed (HopCnt is the maximum
        //   possible value).
        //! \todo Compare the checksum
        if (   packet.size() < sizeof(NetworkControlProtocolHeader)
            || metaData.npHeader.hopCount != NetworkProtocolHeader::maxHopCount)
        {
            // diagnostics.corruptHeader(packet.data());
            packet.dispose();
            return;
        }

        const NetworkControlProtocolHeader header
                = packet.copy_front<NetworkControlProtocolHeader>();

        switch (header.type)
        {
            case NeighborSolicitation::ncpType:
                derived()->onNcpNeighborSolicitation(metaData, packet);
                break;
            case NeighborAdvertisment::ncpType:
                derived()->onNcpNeighborAdvertisment(metaData, packet);
                break;
            default:
                // diagnostics.unknownNcpType(packet);
                packet.dispose();
                break;
        }
    }

private:
    //! Handle a neighbor solicitation.
    //! This method is called upon receiving a neighbor solicitation. The
    //! network protocol's meta data is passed in \p metaData and the \p packet
    //! buffer contains the solicitation's payload.
    //!
    //! The function creates a neighbor advertisment which will be sent back
    //! over the link. If the solicitation has been sent from a valid unicast
    //! address and there is place in the neighbor cache, a cache entry
    //! is generated for the neighbor.
    void onNcpNeighborSolicitation(const ProtocolMetaData& metaData,
                                   BufferBase& packet)
    {
        if (packet.size() < sizeof(NeighborSolicitation))
        {
            packet.dispose();
            return;
        }

        std::cout << "NCP - Received neighbor solicitation" << std::endl;

        const NeighborSolicitation solicitation
            = packet.pop_front<NeighborSolicitation>();

        // Neighbor Solicitations can be sent as unicasts or multicasts. The
        // filtering is done by the target address.
        if (solicitation.targetAddress
              != metaData.networkInterface->networkAddress().hostAddress())
        {
            packet.dispose();
            return;
        }

        // If the sender has transmitted the solicitation with a valid source
        // address, we can create an entry in the neighbor cache right now.
        // This saves another round-trip when we want to send a reply.
        if (!metaData.npHeader.sourceAddress.unspecified())
        {
            Neighbor* neighbor = derived()->nc.find(
                                     metaData.npHeader.sourceAddress);
            if (!neighbor)
            {
                neighbor = derived()->nc.createEntry(
                               metaData.npHeader.sourceAddress,
                               metaData.networkInterface);
            }
            if (neighbor)
            {
                if (NcpOption::SourceLinkLayerAddress* srcLla
                        = NcpOption::find<NcpOption::SourceLinkLayerAddress>(
                            packet.begin(), packet.end()))
                {
                    neighbor->setLinkLayerAddress(srcLla->linkLayerAddress());
                }
            }
        }

        // Build the neighbor advertisment in the same buffer as the
        // solicitation. This saves one buffer allocation.
        packet.clear();

        NetworkControlProtocolMessageBuilder builder(packet);
        builder.createNeighborAdvertisment(solicitation.targetAddress, true);
        if (metaData.networkInterface->linkHasAddresses())
        {
            builder.addTargetLinkLayerAddressOption(
                        metaData.networkInterface->linkLayerAddress());
        }

        NetworkProtocolHeader header;
        header.sourceAddress = HostAddress();//! \todo: npHeader.destinationAddress;
        header.destinationAddress = metaData.npHeader.sourceAddress;
        header.nextHeader = 1;
        header.length = packet.size() + sizeof(NetworkProtocolHeader);
        packet.push_front(header);

        // If the solicitation has been sent from an unspecified host, the
        // advertisment is sent as a broadcast. Otherwise we can use
        // a unicast.
        if (metaData.npHeader.sourceAddress.unspecified())
        {

            //! \todo We should use a non-blocking send call here
            derived()->notify(Event::createSendLinkLocalBroadcast(
                                  metaData.networkInterface, &packet));
        }
        else
        {
            //! \todo We should use a non-blocking send call here
            derived()->notify(Event::createSendRawMessageEvent(&packet));
        }
    }

    //! Handles a neighbor advertisment.
    //! This method is called upon receiving a neighbor advertisment. The
    //! network protocol's meta data is passed in \p metaData and the \p packet
    //! buffer contains the advertisment's payload.
    void onNcpNeighborAdvertisment(const ProtocolMetaData& metaData,
                                   BufferBase& packet)
    {
        if (packet.size() < sizeof(NeighborAdvertisment))
        {
            packet.dispose();
            return;
        }

        std::cout << "NCP - Received neighbor advertisment" << std::endl;

        const NeighborAdvertisment advertisment
            = packet.pop_front<NeighborAdvertisment>();

        // Look up the neighbor to which we have sent the solicitation.
        Neighbor* neighbor = derived()->nc.find(advertisment.targetAddress);
        if (!neighbor)
        {
            //! \todo If there is enough space in the neighbor cache, we might
            //! want to create an entry there.
            packet.dispose();
            return;
        }

        if (NcpOption::TargetLinkLayerAddress* targetLla
            = NcpOption::find<NcpOption::TargetLinkLayerAddress>(
                packet.begin(), packet.end()))
        {
            neighbor->setLinkLayerAddress(targetLla->linkLayerAddress());
        }

        packet.dispose();

        // Send all packets which have been queued until the reachability
        // could be confirmed.
        while (!neighbor->sendQueue().empty())
        {
            BufferBase& buffer = neighbor->sendQueue().front();
            neighbor->sendQueue().pop_front();
            derived()->notify(Event::createSendRawMessageEvent(&buffer));
        }
    }

private:
    KernelT* derived()
    {
        return static_cast<KernelT*>(this);
    }

    const KernelT* derived() const
    {
        return static_cast<const KernelT*>(this);
    }
};

} // namespace uNet

#endif // UNET_NETWORKCONTROLPROTOCOL_HPP
