#ifndef UNET_SIMPLEMESSAGEPROTOCOL_HPP
#define UNET_SIMPLEMESSAGEPROTOCOL_HPP

#include "../config.hpp"
#include "protocol.hpp"
#include "../kernelbase.hpp"

#include "OperatingSystem/OperatingSystem.h"

#include <cstdint>

namespace uNet
{
/*!
Simple Message Protocol

The Simple Message Protocol (SMP) has been designed in the spirit of UDP. It
provides ports to which services can be bound. The SMP handler is merely
a dispatcher, which maps incoming packets from a port number to a service.

\code
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|  Source port  |   Dest port   |              ???              |
+-------+-------+---------------+---------------+---------------+
|                              ???                              |
+---------------+---------------+---------------+---------------+
\endcode

*/

class ReceiveSocketBase;
class SendSocket;
class SimpleMessageProtocol;

//! The header of the Simple Message Protocol.
struct SimpleMessageProtocolHeader
{
    std::uint8_t sourcePort;
    std::uint8_t destinationPort;
    std::uint8_t reserved[6];
};

namespace detail
{
//! A descriptor for a receive connection.
struct ReceiveConnectionDescriptor
{
    ReceiveConnectionDescriptor(ReceiveSocketBase& socket)
        : m_receiveSocket(socket)
    {
    }

    ReceiveSocketBase& m_receiveSocket;

    //! \todo Need a condition variable here instead of this semaphore.
    OperatingSystem::semaphore m_packetSemaphore;

    OperatingSystem::mutex m_mutex;
    BufferQueue m_packetQueue;

    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        descriptor_list_hook_t;
    //! A hook to add this descriptor to a list.
    descriptor_list_hook_t m_desriptorListHook;

    friend class ReceiveSocketBase;
};

} // namespace detail

//! A handle for a receive connection.
//! The ReceiveConnection is a handle for a connection of a receive socket.
class ReceiveConnection
{
private:
    BOOST_MOVABLE_BUT_NOT_COPYABLE(ReceiveConnection)

public:
    //! Creates a receive connection.
    //! Creates a receive connection which wraps the given \p descriptor.
    ReceiveConnection(detail::ReceiveConnectionDescriptor* descriptor)
        : m_descriptor(descriptor)
    {
    }

    // Move constructor.
    ReceiveConnection(BOOST_RV_REF(ReceiveConnection) other)
        :  m_descriptor(other.m_descriptor)
    {
        other.m_descriptor = 0;
    }

    //! Destroys the connection.
    //! Destroys the connection. If it is still open, it will be closed.
    ~ReceiveConnection();

    // Move assignment
    ReceiveConnection& operator= (BOOST_RV_REF(ReceiveConnection) other);

    //! Closes the connection.
    void close();

    //! Waits until a packet is received.
    BufferBase* receive();

    //! Tries to receive a packet within a timeout.
    //! Tries to receive a packet within the timeout period \p d. If no
    //! packet has been received within this time, a null-pointer is
    //! returned.
    template <typename RepT, typename PeriodT>
    BufferBase* try_receive_for(
            const OperatingSystem::chrono::duration<RepT, PeriodT>& d)
    {
        if (!m_descriptor)
            ::uNet::throw_exception(-1); //! \todo system_error

        m_descriptor->m_packetSemaphore.try_wait_for(d);
        OperatingSystem::lock_guard<OperatingSystem::mutex> queueLock(
                    m_descriptor->m_mutex);
        if (m_descriptor->m_packetQueue.empty())
            return 0;
        else
        {
            BufferBase& buffer = m_descriptor->m_packetQueue.front();
            m_descriptor->m_packetQueue.pop_front();
            return &buffer;
        }
    }

private:
    detail::ReceiveConnectionDescriptor* m_descriptor;
};

//! The base class for all receive sockets.
//! ReceiveSocketBase is the base class for all receive sockets.
class ReceiveSocketBase : boost::noncopyable
{
public:
    //! Creates a receive socket.
    //! Creates a receive socket which belongs to the handler \p protocolHandler
    //! and is bound to the port \p localPort.
    ReceiveSocketBase(SimpleMessageProtocol& protocolHandler,
                      std::uint8_t localPort);

    //! Destroys the receive socket.
    ~ReceiveSocketBase();

    //! Waits for a connection.
    //! Listens for incoming connections and blocks until it is established.
    ReceiveConnection accept();

    //! \internal
    void close(detail::ReceiveConnectionDescriptor* descriptor);

    //! Returns the local port.
    //! Returns the local port to which this socket is bound.
    std::uint8_t localPort() const
    {
        return m_localPort;
    }

protected:
    //! Implemented by derived classes to allocate a new descriptor.
    virtual detail::ReceiveConnectionDescriptor* allocateDescriptor() = 0;
    //! Implemented by derived classes to release a descriptor.
    virtual void releaseDescriptor(
            detail::ReceiveConnectionDescriptor* descriptor) = 0;

private:
    //! The protocol handler to which this socket belongs.
    SimpleMessageProtocol& m_protocolHandler;
    //! The local port to which this socket is bound.
    std::uint8_t m_localPort;

    OperatingSystem::mutex m_mutex;

    typedef boost::intrusive::slist<
            detail::ReceiveConnectionDescriptor,
            boost::intrusive::member_hook<
                detail::ReceiveConnectionDescriptor,
                detail::ReceiveConnectionDescriptor::descriptor_list_hook_t,
                &detail::ReceiveConnectionDescriptor::m_desriptorListHook>,
            boost::intrusive::cache_last<false> > descriptor_list_t;
    //! A list of descriptors for open connections.
    descriptor_list_t m_descriptors;

    bool filterPacket(std::uint8_t destinationPort, BufferBase& packet);

    friend class SimpleMessageProtocol;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        socket_list_hook_t;
    socket_list_hook_t m_socketListHook;
};

//! A concrete receive socket.
//! The ReceiveSocket is a concrete receive socket whose maximum number of
//! connections has to be provided via the template parameter
//! \p TMaxNumConnections.
template <unsigned TMaxNumConnections>
class ReceiveSocket : public ReceiveSocketBase
{
public:
    //! Creates a receive socket.
    //! Creates a receive socket which receives messages via the \p protocol
    //! handler on the given \p localPort.
    ReceiveSocket(SimpleMessageProtocol& protocol, std::uint8_t localPort)
        : ReceiveSocketBase(protocol, localPort)
    {
    }

protected:
    //! \reimp
    virtual detail::ReceiveConnectionDescriptor* allocateDescriptor()
    {
        return m_descriptorPool.construct(*this);
    }

    //! \reimp
    void releaseDescriptor(detail::ReceiveConnectionDescriptor* descriptor)
    {
        m_descriptorPool.destroy(descriptor);
    }

private:
    //! A pool for allocating connection descriptors.
    OperatingSystem::object_pool<detail::ReceiveConnectionDescriptor,
                                 TMaxNumConnections> m_descriptorPool;
};

//! A connection to send packets via a send socket.
class SendConnection
{
private:
    //BOOST_MOVABLE_BUT_NOT_COPYABLE(SendConnection)

public:
    void send(BufferBase* packet);

private:
    SendConnection(SendSocket& socket,
                   HostAddress destinationAddress,
                   std::uint8_t destinationPort);

    SendSocket& m_socket;
    HostAddress m_destinationAddress;
    std::uint8_t m_destinationPort;

    //template <unsigned TMaxNumConnections>
    friend class SendSocket;
};

//! The base class for all send sockets.
class SendSocketBase : boost::noncopyable
{

};

//! A send socket.
//template <unsigned TMaxNumConnections>
class SendSocket : public SendSocketBase
{
public:
    SendSocket(SimpleMessageProtocol& protocolHandler, std::uint8_t localPort);

    BufferBase* allocate();
    SendConnection connect(HostAddress destinationAddress,
                           std::uint8_t destinationPort);

    void send(SendConnection& con, BufferBase* packet);

private:
    SimpleMessageProtocol& m_protocolHandler;
    std::uint8_t m_localPort;
};

class SimpleMessageProtocol
{
public:
    //! The value of the "Next header" field in the network protocol.
    static const std::uint8_t headerType = 2;

    SimpleMessageProtocol()
        : m_kernel(0)
    {
    }

    //! Filters incoming packets.
    //! Filters incoming packets by their network protocol \p metaData.
    //! If the network protocol's "Next header" field matches the Simple Message
    //! Protocol type, the method returns \p true.
    bool filter(const ProtocolMetaData& metaData) const
    {
        return metaData.npHeader.nextHeader
                == SimpleMessageProtocol::headerType;
    }

    //! Handles an incoming SMP packet.
    //! Handles the incoming SMP \p packet with an associated network
    //! protocol header \p metaData.
    void receive(const ProtocolMetaData& metaData, BufferBase& packet);

    void send(std::uint8_t sourcePort,
              HostAddress destinationAddress, std::uint8_t destinationPort,
              BufferBase& message);

    //! Sets the associated kernel.
    //! Associates this protocol handler with the given \p kernel.
    void setKernel(KernelBase* kernel)
    {
        m_kernel = kernel;
    }

private:
    void addReceiveSocket(ReceiveSocketBase& socket);
    void removeReceiveSocket(ReceiveSocketBase& socket);

    KernelBase* m_kernel;

    OperatingSystem::mutex m_socketMutex;

    typedef boost::intrusive::slist<
                ReceiveSocketBase,
                boost::intrusive::member_hook<
                    ReceiveSocketBase,
                    ReceiveSocketBase::socket_list_hook_t,
                    &ReceiveSocketBase::m_socketListHook>,
                boost::intrusive::cache_last<false> > receiver_socket_list_t;
    //! A list of receiver sockets which belong to this protocol handler.
    receiver_socket_list_t m_receiveSockets;

    friend class Socket;
    friend class ReceiveSocketBase;
};

} // namespace uNet

#endif // UNET_SIMPLEMESSAGEPROTOCOL_HPP
