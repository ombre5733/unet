#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <boost/array.hpp>
#include <boost/intrusive/slist.hpp>

#include <cstdint>
#include <vector>

class NetworkInterface;

// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
// | . | . | H | H | H | H | D | D | D | D | D | D | D | D | D | D |
// +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
//           ^               ^                                       ^
//           begin           dataBegin                               end
class Buffer
{
public:
    Buffer();

    void advanceData(uint16_t size)
    {
        m_dataBegin += size;
    }

    uint8_t* begin() const
    {
        return m_begin;
    }

    uint16_t capacity() const;

    template <typename TypeT>
    TypeT data_cast()
    {
        if (dataSize() >= sizeof(typename std::remove_pointer<TypeT>::type))
        {
            //! \todo assert(alignof(*TypeT) passt zu m_dataBegin);
            return reinterpret_cast<TypeT>(m_dataBegin);
        }
        else
            return 0;
    }

    void clear();

    void clearHeader()
    {
        m_begin = m_dataBegin;
    }

    void clearData()
    {
        m_end = m_dataBegin;
    }

    uint8_t* dataBegin() const
    {
        return m_dataBegin;
    }

    uint8_t* dataEnd() const
    {
        return m_end;
    }

    uint16_t dataSize() const
    {
        return static_cast<uint16_t>(m_end - m_dataBegin);
    }

    uint8_t* end() const
    {
        return m_end;
    }

    uint16_t headerSize() const
    {
        return static_cast<uint16_t>(m_dataBegin - m_begin);
    }

    NetworkInterface* interface() const
    {
        return m_interface;
    }

    void pop_back(uint16_t size);

    void push_back(const uint8_t* data, uint16_t size);
    void push_front(const uint8_t* data, uint16_t size);

    void setInterface(NetworkInterface* interface);

    uint16_t size() const
    {
        return static_cast<uint16_t>(m_end - m_begin);
    }

private:
    uint8_t* m_begin;
    uint8_t* m_end;
    uint8_t* m_dataBegin;
    // std::vector<ProtocolHandler*> m_protocolStack;
    std::vector<uint8_t> m_data;
    NetworkInterface* m_interface;

    friend class BufferPool;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        slist_hook_t;
    slist_hook_t m_slistHook;
};

typedef boost::intrusive::slist<
        Buffer,
        boost::intrusive::member_hook<
            Buffer,
            Buffer::slist_hook_t,
            &Buffer::m_slistHook>,
        boost::intrusive::cache_last<true> > BufferQueue;

class BufferPool
{
public:
    BufferPool()
    {
        //for (size_t idx = 0; idx < m_buffers.size(); ++idx)
        //    m_freeBuffers.push_back(m_buffers[idx]);
    }

    //boost::shared_ptr<Buffer> allocate();
    static Buffer* allocate()
    {
        return new Buffer;
    }

    //void release(Buffer* buffer);

private:
    //boost::array<Buffer, 16> m_buffers;
    //BufferList m_freeBuffers;
};

#endif // BUFFER_HPP
