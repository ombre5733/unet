#include "buffer.hpp"

#include <cassert>
#include <cstring>

Buffer::Buffer()
    : m_interface(0)
{
    m_data.resize(256, 0);
    m_begin = m_data.data() + 32;
    m_end = m_begin;
}

void Buffer::push_back(const uint8_t *data, uint16_t size)
{
    assert(m_begin + size <= m_data.data() + m_data.size());
    memcpy(m_end, data, size);
    m_end += size;
}

void Buffer::push_front(const uint8_t* data, uint16_t size)
{
    assert(m_data.data() + size <= m_begin);
    m_begin -= size;
    memcpy(m_begin, data, size);
}

void Buffer::setInterface(NetworkInterface* interface)
{
    m_interface = interface;
}
