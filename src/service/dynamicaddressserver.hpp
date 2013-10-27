#ifndef UNET_DYNAMICADDRESSSERVER_HPP
#define UNET_DYNAMICADDRESSSERVER_HPP

#include "../config.hpp"
#include "../networkaddress.hpp"
#include "../protocol/simplemessageprotocol.hpp"

#include <boost/static_assert.hpp>

#include <cstdint>

/*!

# Dynamic Address Service (DAS)   {#DynamicAddressService}

## Messages

### Message layout

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |                 Transaction ID                |
+---------------+---------------+---------------+---------------+
\endcode

### Solicit message

Client -> server

### Advertise message


### Request message

Client -> server in order to get an address for an interface
The client adds a Client Identifier option and an Interface Identifier option.

### Reply message

Server -> client

## Options

\code
0                   1
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-------
|   OptionType  |     Length    | ...
+---------------+---------------+-------
\endcode


### Server Identifier

A unique ID to identify a server.

### Client Identifier

A unique ID to identify a client.

### Interface Identifier

A unique ID to identify an interface of a client. Every interface of a client
must have a unique ID.

*/



namespace uNet
{

namespace dasDetail
{

struct DynamicAddressServiceMessageHeader
{
    std::uint8_t type;
    std::uint8_t transactionId[3];
};

struct DynamicAddressServiceOfferPayload
{
    static const int dasType = 2;

    HostAddress address;
};

struct DynamicAddressServiceOption
{
    std::uint8_t type;
    std::uint8_t length;
};

struct UniqueDeviceIdOption
{
    static const int dasOptionType = 1;

    std::uint8_t id[16];
};

template <typename OptT>
OptT* find(std::uint8_t* begin, std::uint8_t* end)
{
    while (static_cast<std::size_t>(end - begin)
           >= sizeof(DynamicAddressServiceOption))
    {
        const DynamicAddressServiceOption* optHeader
                = reinterpret_cast<const DynamicAddressServiceOption*>(begin);
        if (optHeader->type == OptT::dasOptionType)
            return reinterpret_cast<OptT*>(begin);
        begin += optHeader->length;
    }
    return 0;
}

} // namespace dasDetail

//! A unique device id.
//! The UniqueDeviceId is a wrapper around a unique device id. The byte length
//! of an id is variable but the maximum is 16 bytes (128 bits).
//!
//! The unique device id is needed to "route" broadcast messages. An example
//! is the DAS protocol. As no address has been assigned to the client
//! interface, the messages from the server have to be sent as broadcast.
//! Every message is augmented with the client's unique device ID in order to
//! distinguish the intended receiver and thus to filter incoming traffic.
class UniqueDeviceId
{
public:
    //! The length of the unique device ID.
    std::uint8_t length() const
    {
        return m_length;
    }

    std::uint8_t operator[] (std::uint8_t index) const
    {
        UNET_ASSERT(index < m_length);
        return m_uid[index];
    }

private:
    //! The variable-length ID.
    std::uint8_t m_uid[16];
    //! The length of the ID.
    std::uint8_t m_length;
};

template <unsigned TMaxNumEntries>
class DynamicAddressServer
{
    BOOST_STATIC_ASSERT(TMaxNumEntries > 0);

public:
    explicit DynamicAddressServer(SimpleMessageProtocol& smp);



private:
    struct Entry
    {
        std::uint8_t m_uidLength;
        std::uint8_t m_uid[16];
        HostAddress m_address;
    };

    std::size_t m_numEntries;
    Entry m_entries[TMaxNumEntries];
};

template <unsigned TMaxNumInterfaces>
class DynamicAddressClient
{
public:
    explicit DynamicAddressClient(SimpleMessageProtocol& protocol);

    //! Adds a network interface to the list of configurable interfaces.
    //! Adds the network interface \p ifc to the list of configurable
    //! interfaces. The client is responsible for assigning a network
    //! address to the interface.
    void addInterface(NetworkInterface* ifc);

    void setUniqueDeviceId(const UniqueDeviceId& id);

private:
    //! A collection of data needed by the client for one interface.
    struct InterfaceData
    {
        enum State
        {
            Initial,
            WaitForOffer,
            Acknowledged,
            WaitForConfirmation,
            Confirmed
        };

        InterfaceData()
            : m_state(Initial),
              m_interface(0)
        {
        }

        InterfaceData(NetworkInterface* ifc)
            : m_state(Initial),
              m_interface(ifc)
        {
        }

        //! The interface's configuration state.
        State m_state;
        //! The interface.
        NetworkInterface* m_interface;

        // A event for timeouts.
        //Event m_event;
    };

    //! The list of interfaces which are configured by this client.
    InterfaceData m_interfaceData[TMaxNumInterfaces];
    UniqueDeviceId m_udid;

    SimpleMessageProtocol& m_protocol;
};

template <unsigned TMaxNumInterfaces>
DynamicAddressClient::DynamicAddressClient(SimpleMessageProtocol& protocol)
    : m_protocol(protocol)
{
}

template <unsigned TMaxNumInterfaces>
void DynamicAddressClient::addInterface(NetworkInterface *ifc)
{
    for (std::size_t idx = 0; idx < TMaxNumInterfaces; ++idx)
    {
        if (m_interfaceData[idx].m_interface == 0)
        {
            m_interfaceData[idx] = InterfaceData(ifc);
            return;
        }
    }

    ::uNet::throw_exception(-1);
}

template <unsigned TMaxNumInterfaces>
void DynamicAddressClient::onInterfaceEnable()
{
    SendSocket sendSocket(m_protocol, 2);
    SendConnection connection = sendSocket.connect(serverAddress, 1);
    BufferBase* buffer = sendSocket.allocate();

    DynamicAddressMessageBuilder builder(*buffer);
    builder.createRequest();

    connection.send(*buffer);
}

template <unsigned TMaxNumInterfaces>
void DynamicAddressClient::listen()
{
    ReceiveSocket<1> receiveSocket(m_protocol, 2);
    //! \todo Merge all incoming messages into the one and only socket connection.
    ReceiveConnection connection = receiveSocket.accept();
    while (1)
    {
        BufferBase* buffer = connection.receive();

        if (buffer->size() < sizeof(DynamicAddressServiceMessageHeader))
        {
            buffer->dispose();
            continue;
        }

        dasDetail::DynamicAddressServiceMessageHeader header
                = buffer->pop_front<dasDetail::DynamicAddressServiceMessageHeader>();

        switch (header.type)
        {
            case dasDetail::DynamicAddressServiceOfferPayload::dasType:
                onOffer(header, *buffer);
            default:
                buffer->dispose();
        }
    }
}

template <unsigned TMaxNumInterfaces>
void DynamicAddressClient::onOffer(
        const dasDetail::DynamicAddressServiceMessageHeader& header,
        BufferBase& message)
{
    if (message.size() < dasDetail::DynamicAddressServiceOfferPayload)
    {
        message.dispose();
        return;
    }

    dasDetail::DynamicAddressServiceOfferPayload offer
            = message.pop_front<dasDetail::DynamicAddressServiceOfferPayload>();
    if (   offer.address.unspecified()
        || offer.address.multicast())
    {
        message.dispose();
        return;
    }

    dasDetail::UniqueDeviceIdOption* udid
            = dasDetail::find<dasDetail::UniqueDeviceIdOption>(message.begin(),
                                                               message.end());
    if (!udid)
    {
        message.dispose();
        return;
    }

    for (int idx = 0; idx < TMaxNumInterfaces; ++idx)
    {
        if (m_interfaceData[idx].m_interface != ifc)
            continue;

        m_interfaceData[idx].m_state

        break;
    }

    message.dispose();
}


struct DynamicAddressServiceRequest
{
};

struct DynamicAddressServiceOffer
{
};

struct DynamicAddressServiceConfirmation
{
};

//! A helper to build DAS messages in a buffer.
class DynamicAddressMessageBuilder
{
public:
    DynamicAddressMessageBuilder(BufferBase& buffer)
        : m_buffer(buffer)
    {
    }

    void addUniqueDeviceIdOption(const UniqueDeviceId& udid)
    {
        m_buffer.push_back(1);                 // OptionType.
        m_buffer.push_back(udid.length() + 2); // Length.
        //! \todo BufferBase should really take iterators.
        for (int idx = 0; idx < udid.length(); ++idx)
            m_buffer.push_back(udid[idx]);
    }

    void createRequest()
    {
        m_buffer.push_back(??);
    }

    void createOffer()
    {
    }

    void createAcknowledge()
    {
    }

    void createConfirmation()
    {
    }

private:
    //! The buffer in which the messages will be built.
    BufferBase& m_buffer;
};

} // namespace uNet

#endif // UNET_DYNAMICADDRESSSERVER_HPP
