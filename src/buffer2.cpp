#include "buffer2.hpp"

BufferHandle::BufferHandle(Buffer2* buffer)
    : m_buffer(buffer)
{
    if (buffer)
        buffer->acquire();
}

BufferHandle::~BufferHandle()
{
    if (m_buffer)
        m_buffer->release();
}
