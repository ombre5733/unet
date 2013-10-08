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

#if 0
struct SMP
{
    template <typename TKernel>
    struct handler
    {
        typedef typename SMP_Handler<TKernel> type;
    };

    template <typename TKernel>
    struct socket
    {
    };
};
#endif

template <typename TProtocol>
struct protocol_traits;

//! The header of the Simple Message Protocol.
struct SimpleMessageProtocolHeader
{
    std::uint8_t sourcePort;
    std::uint8_t destinationPort;
    std::uint8_t reserved[6];
};

class SimpleMessageProtocol;

#if 0
class SocketBase
{
public:
    virtual ~SocketBase() {}
    virtual void send(BufferBase& buffer) = 0;
};

template <unsigned TConnectionCount>
class Socket : public SocketBase : boost::noncopyable
{

};

//! A socket communicator.
//! A SocketCommunicator is an object for communicating over a socket. It
//! is created whenever a socket establishes a connection. The classic
//! pedant is the file descriptor returned by accept() or connect().
class SocketCommunicator : boost::noncopyable
{
public:
    explicit SocketCommunicator(SocketBase& socket);


private:
    //! The socket to which this communicator belongs.
    SocketBase& m_socket;
};

void server()
{
    Socket<5> s(protocol); // 5 parallel connections to this port
    s.bind(23);
    s.listen();
    while (1)
    {
        SocketCommunicator c = s.accept();
        BufferBase* b = c.receive(); // c.try_receive() / c.try_receive_for(duration)
        // c.close();
    }
}

void client()
{
    Socket<> s(protocol);
    s.bind(21);
    SocketCommunicator c = s.connect(serverAddress, 23);
    BufferBase* b = c.allocate(); // c.try_allocate() / c.try_allocate_for()
    b->push_back(0x0815);
    c.send(b);
    // c.close();
}

#endif

//! A socket.
class Socket
{
public:
    enum State
    {
        Disconnected,
        Bound,
        Listening,
        Connecting,
        Receiving,
        Sending,
        Closing
    };

    explicit Socket(SimpleMessageProtocol& protocol)
        : m_protocolHandler(protocol),
          m_addedToProtocolHandler(false),
          m_state(Disconnected),
          m_localPort(0)
    {
    }

    ~Socket();

    void accept();

    //! Binds the socket to a local port.
    //! Binds the socket to the local \p port. A bound socket can then be
    //! used for receiving packets by calling listen() or it can connect
    //! to a remote port via connect().
    void bind(std::uint8_t port);

    //! Listens for packets.
    //! Turns this socket into an incoming one. It will listen on the port
    //! which has been set with bind() for packets. The incoming packets are
    //! queued until they are fetched via a call to receive().
    void listen();

    //! Connects to a remote port.
    //! Connects this port to a remote port with number \p destinationPort on
    //! the host with the address \p destinationAddress.
    void connect(HostAddress destinationAddress, std::uint8_t destinationPort);

    BufferBase* receive();

    BufferBase* try_receive();

    BufferBase* try_receive_for();

    BufferBase* allocate();

    void send(BufferBase* packet);

private:
    OperatingSystem::mutex m_mutex;
    //! The protocol handler to which this socket belongs.
    SimpleMessageProtocol& m_protocolHandler;
    bool m_addedToProtocolHandler;
    State m_state;
    OperatingSystem::semaphore m_packetSemaphore;
    BufferQueue m_packetQueue;
    HostAddress m_localAddress;
    std::uint8_t m_localPort;

    bool filterPacket(std::uint8_t destinationPort, BufferBase& packet);

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        protocol_list_hook_t;
    protocol_list_hook_t m_protocolListHook;

    friend class SimpleMessageProtocol;
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

    /*
    virtual void send(Service* service, CrossLayerSendData& metaData,
                      BufferBase& message)
    {
    }

    void send(std::uint8_t sourcePort,
              HostAddress destinationAddress, std::uint8_t destinationPort,
              BufferBase& message)
    {
        if (m_kernel)
            m_kernel->send(destinationAddress, SimplePortProtocol::headerType,
                           message);
    }
*/
    //! Sets the associated kernel.
    //! Associates this protocol handler with the given \p kernel.
    void setKernel(KernelBase* kernel)
    {
        m_kernel = kernel;
    }

private:
    void addSocket(Socket &socket);
    void removeSocket(Socket& socket);

    KernelBase* m_kernel;

    OperatingSystem::mutex m_socketMutex;

    typedef boost::intrusive::slist<
            Socket,
            boost::intrusive::member_hook<
                Socket,
                Socket::protocol_list_hook_t,
                &Socket::m_protocolListHook>,
            boost::intrusive::cache_last<false> > socket_list_t;
    //! A list of sockets which are attached to this protocol handler.
    socket_list_t m_socketList;

    friend class Socket;
};

} // namespace uNet

#endif // UNET_SIMPLEMESSAGEPROTOCOL_HPP
