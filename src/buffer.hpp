#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <cstdint>
#include <vector>

class NetworkInterface;

// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
// |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
// +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//           ^              ^                       ^
//           header         data                    stop
class Buffer
{
public:
    Buffer();

    uint8_t* begin() const
    {
        return m_begin;
    }

    uint8_t* end() const
    {
        return m_end;
    }

    NetworkInterface* interface() const
    {
        return m_interface;
    }

    void push_front(const uint8_t* data, uint16_t size);

    uint16_t size() const
    {
        return m_end - m_begin;
    }

    /*
    const uint8_t* data() const;

    NetworkInterface* interface() const; // ??
    uint16_t length() const;

    void prepend(uint16_t header);
    void prepend(uint32_t header);
    */

private:
    uint8_t* m_begin;
    uint8_t* m_end;
    // std::vector<ProtocolHandler*> m_protocolStack;
    std::vector<uint8_t> m_data;
    NetworkInterface* m_interface;
};

#endif // BUFFER_HPP
