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

void Buffer::push_front(const uint8_t* data, uint16_t size)
{
    assert(m_data.data() + size <= m_begin);
    m_begin -= size;
    memcpy(m_begin, data, size);
}
