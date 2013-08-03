#ifndef NETWORKCONTROLPROTOCOL_HPP
#define NETWORKCONTROLPROTOCOL_HPP

/*!
Network Control Protocol

All messages start with the following header:

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+

The Type field encodes the NCP message type. The Code field can be
used for sub-typing and is set to zero, if the Type does not have
sub-types.


Neighbor solicitation message
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+
|        Target address         |           reserved            |
+---------------------------------------------------------------+

NCP fields:
    Type    1
    Code    0


Neighbor advertisment message
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |           Checksum            |
+---------------+---------------+---------------+---------------+
|        Target address         |S|         reserved            |
+---------------------------------------------------------------+

NCP fields:
    Type    2
    Code    0
*/

#include "buffer.hpp"
#include "networkaddress.hpp"
#include "linklayeraddress.hpp"

#include <cstdint>

struct NetworkControlProtocolHeader
{
    NetworkControlProtocolHeader(uint8_t t, uint8_t c = 0)
        : type(t),
          code(c)
    {
    }

    uint8_t type;
    uint8_t code;
    uint16_t checksum;
};

struct NeighborSolicitation
{
    NeighborSolicitation()
        : header(1),
          reserved(0)
    {
    }

    NetworkControlProtocolHeader header;
    uint16_t targetAddress;
    uint16_t reserved;
};

struct NeighborAdvertisment
{
    NeighborAdvertisment()
        : header(2),
          reserved(0)
    {
    }

    NetworkControlProtocolHeader header;
    uint16_t targetAddress;
    uint16_t reserved;
};

struct NetworkControlProtocolOption
{
    const static int unitByteSize = 4;

    NetworkControlProtocolOption(uint8_t _type, uint8_t _length)
        : type(_type),
          length(_length)
    {
    }

    uint16_t byteSize() const
    {
        return length * unitByteSize;
    }

    uint8_t type;
    uint8_t length;
};


namespace NcpOption
{

struct SourceLinkLayerAddress : public NetworkControlProtocolOption
{
    const static int ncpOptionType = 1;

    SourceLinkLayerAddress()
        : NetworkControlProtocolOption(
              ncpOptionType,
              (sizeof(*this) + unitByteSize-1) / unitByteSize)
    {
    }

    LinkLayerAddress linkLayerAddress;
};

struct TargetLinkLayerAddress : public NetworkControlProtocolOption
{
    const static int ncpOptionType = 2;

    TargetLinkLayerAddress()
        : NetworkControlProtocolOption(
              ncpOptionType,
              (sizeof(*this) + unitByteSize-1) / unitByteSize)
    {
    }

    LinkLayerAddress linkLayerAddress;
};

template <typename OptT>
OptT* find(uint8_t* begin, uint8_t* end)
{
    while (static_cast<size_t>(end - begin) >= sizeof(NetworkControlProtocolOption))
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

class NetworkControlProtocolMessageBuilder
{
public:
    NetworkControlProtocolMessageBuilder(Buffer& buffer)
        : m_buffer(buffer)
    {
    }

    void createNeighborAdvertisment(HostAddress targetAddress)
    {
        NeighborAdvertisment advertisment;
        advertisment.targetAddress = targetAddress;
        m_buffer.push_back((uint8_t*)&advertisment, sizeof(advertisment));
    }

    void createNeighborSolicitation(HostAddress targetAddress)
    {
        NeighborSolicitation solicitation;
        solicitation.targetAddress = targetAddress;
        m_buffer.push_back((uint8_t*)&solicitation, sizeof(solicitation));
    }

    void addSourceLinkLayerAddressOption(LinkLayerAddress linkLayerAddress)
    {
        NcpOption::SourceLinkLayerAddress llaOpt;
        llaOpt.linkLayerAddress = linkLayerAddress;
        m_buffer.push_back((uint8_t*)&llaOpt, sizeof(llaOpt));
    }

    void addTargetLinkLayerAddressOption(LinkLayerAddress linkLayerAddress)
    {
        NcpOption::TargetLinkLayerAddress llaOpt;
        llaOpt.linkLayerAddress = linkLayerAddress;
        m_buffer.push_back((uint8_t*)&llaOpt, sizeof(llaOpt));
    }

private:
    Buffer& m_buffer;
};



class UnetHeader;

template <typename DerivedT>
class NetworkControlProtocol
{
public:
    static const int headerType = 1;

    void createNeighborSolictiation()
    {
        /*
        Buffer* b = BufferPool::allocate();
        UnetHeader header;
        header.sourceAddress = interface->networkAddress().hostAddress();
        header.destinationAddress = HostAddress::multicastAddress();
        header.nextHeader = 1;
        b->push_back((uint8_t*)&header, sizeof(header));

        NeighborSolicitation solicitation;
        solicitation.targetAddress = destAddr;
        b->push_back((uint8_t*)&solicitation, sizeof(solicitation));
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

        std::cout << "NCP - header type = " << (uint16_t)header->type << std::endl;
        switch (header->type)
        {
            case 1:
                derived()->onNcpNeighborSolicitation(
                            reinterpret_cast<const NeighborSolicitation*>(packet.dataBegin()),
                            packet);
                break;
            case 2:
                derived()->onNcpNeighborAdvertisment(packet);
                break;
            default:
                // diagnostics.unknownNcpType(packet);
                break;
        }
    }

    void onNcpNeighborSolicitation(const NeighborSolicitation*, Buffer&)
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

#endif // NETWORKCONTROLPROTOCOL_HPP
