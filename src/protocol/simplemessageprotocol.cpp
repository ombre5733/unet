#include "simplemessageprotocol.hpp"

namespace uNet
{
// ----=====================================================================----
//     Socket
// ----=====================================================================----

Socket::~Socket()
{
    if (m_addedToProtocol)
        m_protocol.removeSocket(*this);
}

void Socket::accept()
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    m_state = Connected;
}

void Socket::listen()
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    m_protocol.addSocket(*this);
    m_addedToProtocol = true;
}

BufferBase* Socket::receive()
{
    m_packetSemaphore.wait();
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    BufferBase* packet = m_lastPacket;
    m_lastPacket = 0;
    return packet;
}

void Socket::receiveFromProtocol(BufferBase& packet)
{
    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
    if (m_state != Connected)
        packet.dispose();
    else
    {
        m_lastPacket = &packet;
        m_packetSemaphore.post();
    }
}

// ----=====================================================================----
//     SimpleMessageProtocol
// ----=====================================================================----

void SimpleMessageProtocol::receive(std::uint8_t, BufferBase& packet)
{
    if (packet.size() < sizeof(SimpleMessageProtocolHeader))
    {
        packet.dispose();
        return;
    }
    std::cout << "This is a SMP message" << std::endl;

    const SimpleMessageProtocolHeader* header
            = reinterpret_cast<const SimpleMessageProtocolHeader*>(
                  packet.begin());
    packet.moveBegin(sizeof(SimpleMessageProtocolHeader));

    std::cout << "{SMP} src: " << int(header->sourcePort)
              << " dest: " << int(header->destinationPort) << std::endl;

    OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_socketMutex);
    if (m_socketList.empty())
    {
        packet.dispose();
        return;
    }

    m_socketList.front().receiveFromProtocol(packet);
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

} // namespace uNet
