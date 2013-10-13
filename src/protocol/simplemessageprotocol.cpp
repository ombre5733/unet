#include "simplemessageprotocol.hpp"

namespace uNet
{
// ----=====================================================================----
//     ReceiveConnection
// ----=====================================================================----

ReceiveConnection::~ReceiveConnection()
{
    close();
}

ReceiveConnection& ReceiveConnection::operator= (
        BOOST_RV_REF(ReceiveConnection) other)
{
    close();
    m_descriptor = other.m_descriptor;
    other.m_descriptor = 0;
    return *this;
}

void ReceiveConnection::close()
{
    if (m_descriptor)
    {
        m_descriptor->m_receiveSocket.close(m_descriptor);
        m_descriptor = 0;
    }
}

BufferBase* ReceiveConnection::receive()
{
    if (!m_descriptor)
        ::uNet::throw_exception(-1); //! \todo system_error

    m_descriptor->m_packetSemaphore.wait();
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

// ----=====================================================================----
//     ReceiveSocketBase
// ----=====================================================================----

ReceiveSocketBase::ReceiveSocketBase(SimpleMessageProtocol& protocolHandler,
                                     std::uint8_t localPort)
    : m_protocolHandler(protocolHandler),
      m_localPort(localPort)
{
    m_protocolHandler.addReceiveSocket(*this);
}

ReceiveSocketBase::~ReceiveSocketBase()
{
    m_protocolHandler.removeReceiveSocket(*this);
}

ReceiveConnection ReceiveSocketBase::accept()
{
    detail::ReceiveConnectionDescriptor* desc = allocateDescriptor();
    if (!desc)
        ::uNet::throw_exception(-1); //! \todo system_error

    OperatingSystem::lock_guard<OperatingSystem::mutex> listLock(m_mutex);
    m_descriptors.push_front(*desc);

    return ReceiveConnection(desc);
}

void ReceiveSocketBase::close(detail::ReceiveConnectionDescriptor* descriptor)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> listLock(m_mutex);
    m_descriptors.erase(m_descriptors.iterator_to(*descriptor));
    releaseDescriptor(descriptor);
}

// -----------------------------------------------------------------------------
// Private methods
// -----------------------------------------------------------------------------

bool ReceiveSocketBase::filterPacket(std::uint8_t destinationPort,
                                     BufferBase& packet)
{
    if (m_localPort != destinationPort)
        return false;

    OperatingSystem::lock_guard<OperatingSystem::mutex> listLock(m_mutex);

    for (descriptor_list_t::iterator iter = m_descriptors.begin(),
                                 end_iter = m_descriptors.end();
         iter != end_iter; ++iter)
    {
        OperatingSystem::lock_guard<OperatingSystem::mutex> queueLock(
                    iter->m_mutex);
        iter->m_packetQueue.push_back(packet);
        iter->m_packetSemaphore.post();
        return true;
    }

    packet.dispose();
    return true;
}

// ----=====================================================================----
//     SendConnection
// ----=====================================================================----

SendConnection::SendConnection(SendSocket& socket,
                               HostAddress destinationAddress,
                               std::uint8_t destinationPort)
    : m_socket(socket),
      m_destinationAddress(destinationAddress),
      m_destinationPort(destinationPort)
{
}

void SendConnection::send(BufferBase* packet)
{
    m_socket.send(*this, packet);
}

// ----=====================================================================----
//     SendSocket
// ----=====================================================================----

SendSocket::SendSocket(SimpleMessageProtocol &protocolHandler,
                       std::uint8_t localPort)
    : m_protocolHandler(protocolHandler),
      m_localPort(localPort)
{
}

SendConnection SendSocket::connect(HostAddress destinationAddress,
                                   std::uint8_t destinationPort)
{
    return SendConnection(*this, destinationAddress, destinationPort);
}

void SendSocket::send(SendConnection &con, BufferBase *packet)
{
    m_protocolHandler.send(
                m_localPort, con.m_destinationAddress,
                con.m_destinationPort, *packet);
}

// ----=====================================================================----
//     SimpleMessageProtocol
// ----=====================================================================----

void SimpleMessageProtocol::addReceiveSocket(ReceiveSocketBase& socket)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);

    if (m_receiveSockets.empty())
    {
        m_receiveSockets.push_front(socket);
        return;
    }

    receiver_socket_list_t::iterator prev_iter = m_receiveSockets.end();
    for (receiver_socket_list_t::iterator iter = m_receiveSockets.begin(),
                                      end_iter = m_receiveSockets.end();
         iter != end_iter; ++iter)
    {
        // No two sockets can bind to the same local port.
        if (iter->localPort() == socket.localPort())
        {
            ::uNet::throw_exception(-1); // system_error
        }
        else if (iter->localPort() < socket.localPort())
        {
            prev_iter = iter;
            continue;
        }

        // The local port of the socket referenced by iter is higher than the
        // local port of the new socket. Insert the socket just before the
        // current position.
        if (prev_iter == m_receiveSockets.end())
            m_receiveSockets.push_front(socket);
        else
            m_receiveSockets.insert_after(prev_iter, socket);
        break;
    }
}

void SimpleMessageProtocol::removeReceiveSocket(ReceiveSocketBase& socket)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);
    m_receiveSockets.erase(m_receiveSockets.iterator_to(socket));
}

void SimpleMessageProtocol::receive(const ProtocolMetaData& /*metaData*/,
                                    BufferBase& packet)
{
    if (packet.size() < sizeof(SimpleMessageProtocolHeader))
    {
        packet.dispose();
        return;
    }
    std::cout << "This is a SMP message" << std::endl;

    const SimpleMessageProtocolHeader header
            = packet.pop_front<SimpleMessageProtocolHeader>();

    std::cout << "{SMP} src: " << int(header.sourcePort)
              << " dest: " << int(header.destinationPort) << std::endl;

    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);
    if (m_receiveSockets.empty())
    {
        packet.dispose();
        return;
    }

    for (receiver_socket_list_t::iterator iter = m_receiveSockets.begin(),
                                      end_iter = m_receiveSockets.end();
         iter != end_iter; ++iter)
    {
        if (iter->filterPacket(header.destinationPort, packet))
            return;
    }
    packet.dispose();
}

void SimpleMessageProtocol::send(
        std::uint8_t sourcePort, HostAddress destinationAddress,
        std::uint8_t destinationPort, BufferBase& message)
{
    if (!m_kernel)
    {
        message.dispose();
        return;
    }

    SimpleMessageProtocolHeader header;
    header.sourcePort = sourcePort;
    header.destinationPort = destinationPort;
    message.push_front(header);
    m_kernel->send(destinationAddress, SimpleMessageProtocol::headerType,
                   message);
}

} // namespace uNet
