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
\endcode

NCP fields:
    Type    1
    Code    0

The <tt>Target address</tt> is the address of the device which is solicited.

Only the <tt>Source link-layer address</tt> is allowed as an option in the
neighbor solicitation message.


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
\endcode

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

NCP options can be appended to an NCP message. The following options are
available:

Source link-layer address option

Target link-layer address option

*/

#include "config.hpp"

#include "buffer.hpp"
#include "event.hpp"
#include "networkaddress.hpp"
#include "networkheader.hpp"
#include "linklayeraddress.hpp"

#include <cstdint>

namespace uNet
{

struct NetworkControlProtocolHeader
{
    NetworkControlProtocolHeader(std::uint8_t t, std::uint8_t c = 0)
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
    std::uint16_t targetAddress;
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
    std::uint16_t targetAddress;
    bool solicited : 1;
    std::uint16_t reserved : 15;
};

namespace NcpOption
{

struct NetworkControlProtocolOption
{
    static const int unitByteSize = 4;

    NetworkControlProtocolOption(std::uint8_t _type, std::uint8_t _length)
        : type(_type),
          length(_length)
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

struct SourceLinkLayerAddress : public NetworkControlProtocolOption
{
    static const int ncpOptionType = 1;

    SourceLinkLayerAddress()
        : NetworkControlProtocolOption(
              ncpOptionType,
              (sizeof(*this) + unitByteSize-1) / unitByteSize)
    {
    }

    std::uint8_t linkLayerAddress[sizeof(LinkLayerAddress)];
};

struct TargetLinkLayerAddress : public NetworkControlProtocolOption
{
    static const int ncpOptionType = 2;

    TargetLinkLayerAddress()
        : NetworkControlProtocolOption(
              ncpOptionType,
              (sizeof(*this) + unitByteSize-1) / unitByteSize)
    {
    }

    std::uint8_t linkLayerAddress[sizeof(LinkLayerAddress)];
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
    NetworkControlProtocolMessageBuilder(BufferBase& buffer)
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
        NcpOption::SourceLinkLayerAddress llaOpt;
        //llaOpt.linkLayerAddress = linkLayerAddress;
        while (1);
        m_buffer.push_back(llaOpt);
    }

    //! Adds a target link-layer address option.
    void addTargetLinkLayerAddressOption(LinkLayerAddress linkLayerAddress)
    {
        NcpOption::TargetLinkLayerAddress llaOpt;
        //llaOpt.linkLayerAddress = linkLayerAddress;
        while (1);
        m_buffer.push_back(llaOpt);
    }

private:
    //! The buffer in which the message will be built.
    BufferBase& m_buffer;
};


#if 0
class UnetHeader;

template <typename DerivedT>
class NetworkControlProtocol
{
public:
    static const int headerType = 1;

    bool accepts(int nextHeaderType) const
    {
        return nextHeaderType == NetworkControlProtocol::headerType;
    }

    void handle(const char* data)
    {
        std::cout << "Ncp protocol - " << data << std::endl;
    }

    //NetworkControlProtocol(Kernel* kernel);

    void createNeighborSolicitation()
    {
        /*
        Buffer* b = BufferPool::allocate();
        UnetHeader header;
        header.sourceAddress = interface->networkAddress().hostAddress();
        header.destinationAddress = HostAddress::multicastAddress();
        header.nextHeader = 1;
        b->push_back((std::uint8_t*)&header, sizeof(header));

        NeighborSolicitation solicitation;
        solicitation.targetAddress = destAddr;
        b->push_back((std::uint8_t*)&solicitation, sizeof(solicitation));
        */
    }

    void dispatch(const UnetHeader* netHeader, Buffer& packet)
    {
        std::cout << "NCP - dispatching packet of size " << packet.dataSize() << std::endl;
        if (packet.dataSize() < sizeof(NetworkControlProtocolHeader))
        {
            // diagnostics.corruptHeader(packet.data());
            return;
        }

        const NetworkControlProtocolHeader* header
                = reinterpret_cast<const NetworkControlProtocolHeader*>(packet.dataBegin());

        std::cout << "NCP - header type = " << (std::uint16_t)header->type << std::endl;
        switch (header->type)
        {
            case 1:
                derived()->onNcpNeighborSolicitation(packet);
                break;
            case 2:
                derived()->onNcpNeighborAdvertisment(packet);
                break;
            default:
                // diagnostics.unknownNcpType(packet);
                break;
        }
    }

    void onNcpNeighborSolicitation(Buffer&)
    {
        std::cout << "NCP - Received neighbor solicitation" << std::endl;
    }

    void onNcpNeighborAdvertisment(Buffer& packet)
    {
        std::cout << "NCP - Received neighbor advertisment" << std::endl;
    }

private:
    DerivedT* derived()
    {
        return static_cast<DerivedT*>(this);
    }

    const DerivedT* derived() const
    {
        return static_cast<const DerivedT*>(this);
    }
};
#endif

template <typename KernelT>
class NcpHandler
{
public:
    void handle(const NetworkHeader* netHeader, BufferBase& message)
    {
        // Ignore malformed messages.
        if (message.size() < sizeof(NetworkControlProtocolHeader))
        {
            // diagnostics.corruptHeader(packet.data());
            return;
        }

        const NetworkControlProtocolHeader* header
                = reinterpret_cast<const NetworkControlProtocolHeader*>(message.begin());

        switch (header->type)
        {
            case NeighborSolicitation::ncpType:
                derived()->onNcpNeighborSolicitation(netHeader, message);
                break;
                /*
            case NeighborAdvertisment::ncpType:
                derived()->onNcpNeighborAdvertisment(packet);
                break;*/
            default:
                // diagnostics.unknownNcpType(packet);
                break;
        }
    }

    void onNcpNeighborSolicitation(const NetworkHeader* netHeader,
                                   BufferBase& message)
    {
        std::cout << "NCP - Received neighbor solicitation" << std::endl;

        const NeighborSolicitation* solicitation
            = reinterpret_cast<const NeighborSolicitation*>(message.begin());

        BufferBase* buffer = derived()->allocateBuffer();

        NetworkControlProtocolMessageBuilder builder(*buffer);
        builder.createNeighborAdvertisment(solicitation->targetAddress, true);
        //! \todo: Add the link-layer address of the sender
        //builder.addTargetLinkLayerAddressOption();

        NetworkHeader header;
        header.sourceAddress = netHeader->destinationAddress;
        header.destinationAddress = netHeader->sourceAddress;
        header.nextHeader = 1;
        buffer->push_front(header);

        derived()->notify(Event::createSendRawMessageEvent(buffer));
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
