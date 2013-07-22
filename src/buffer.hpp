#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <boost/array.hpp>
#include <boost/intrusive/slist.hpp>

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

    uint16_t capacity() const;

    uint8_t* end() const
    {
        return m_end;
    }

    NetworkInterface* interface() const
    {
        return m_interface;
    }

    void push_back(const uint8_t* data, uint16_t size);
    void push_front(const uint8_t* data, uint16_t size);

    void setInterface(NetworkInterface* interface);

    uint16_t size() const
    {
        return m_end - m_begin;
    }

private:
    uint8_t* m_begin;
    uint8_t* m_end;
    // std::vector<ProtocolHandler*> m_protocolStack;
    std::vector<uint8_t> m_data;
    NetworkInterface* m_interface;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        slist_hook_t;
    slist_hook_t m_slistHook;
};

class BufferPool
{
public:
    BufferPool()
    {
        for (size_t idx = 0; idx < m_buffers.size(); ++idx)
            m_freeBuffers.push_back(m_buffers[idx]);
    }

    //boost::shared_ptr<Buffer> allocate();

    //void release(Buffer* buffer);

private:
    typedef boost::intrusive::member_hook<
            Buffer,
            Buffer::slist_hook_t,
            &Buffer::m_slistHook> buffer_hook_options;
    typedef boost::intrusive::slist<
            Buffer,
            buffer_hook_options,
            boost::intrusive::cache_last<true> > BufferList;

    boost::array<Buffer, 16> m_buffers;
    BufferList m_freeBuffers;
};

#endif // BUFFER_HPP
