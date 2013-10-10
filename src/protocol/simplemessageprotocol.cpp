#include "simplemessageprotocol.hpp"

namespace uNet
{
// ----=====================================================================----
//     Socket
// ----=====================================================================----

Socket::~Socket()
{
    if (m_addedToProtocolHandler)
        m_protocolHandler.removeSocket(*this);

    while (!m_packetQueue.empty())
    {
        BufferBase& packet = m_packetQueue.front();
        m_packetQueue.pop_front();
        packet.dispose();
    }
}

void Socket::accept()
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    if (m_state != Listening)
        ::uNet::throw_exception(-1); //! \todo system_error
    m_state = Receiving;
}

void Socket::bind(std::uint8_t port)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    m_localPort = port;
    m_state = Bound;
}

void Socket::connect(HostAddress destinationAddress,
                     std::uint8_t destinationPort)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    m_remoteAddress = destinationAddress;
    m_remotePort = destinationPort;
}

void Socket::listen()
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    if (m_state != Bound)
        ::uNet::throw_exception(-1); //! \todo system_error
    m_protocolHandler.addSocket(*this);
    m_addedToProtocolHandler = true;
    m_state = Listening;
}

BufferBase* Socket::receive()
{
    m_packetSemaphore.wait();
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    BufferBase& packet = m_packetQueue.front();
    m_packetQueue.pop_front();
    return &packet;
}

void Socket::send(BufferBase& message)
{
    m_protocolHandler.send(m_localPort, m_remoteAddress, m_remotePort, message);
}

bool Socket::filterPacket(std::uint8_t destinationPort, BufferBase& packet)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    if (m_state != Receiving || destinationPort != m_localPort)
        return false;

    m_packetQueue.push_back(packet);
    m_packetSemaphore.post();
    return true;
}

// ----=====================================================================----
//     SimpleMessageProtocol
// ----=====================================================================----

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
    if (m_socketList.empty())
    {
        packet.dispose();
        return;
    }

    for (socket_list_t::iterator iter = m_socketList.begin(),
                             end_iter = m_socketList.end();
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

void SimpleMessageProtocol::addSocket(Socket& socket)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);
    m_socketList.push_front(socket);
}

void SimpleMessageProtocol::removeSocket(Socket& socket)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);
    m_socketList.erase(m_socketList.s_iterator_to(socket));
}

//void SimpleMessageProtocol::send()

} // namespace uNet
