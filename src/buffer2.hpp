#ifndef BUFFER2_HPP
#define BUFFER2_HPP

#include <boost/intrusive/slist.hpp>
#include <boost/type_traits.hpp>

#include <cstdint>
#include <cstring>

class Buffer2
{
public:
    static const int BUFFER_SIZE = 256;

    Buffer2()
    {
        m_referenceCount = 1;
        m_begin = m_end = static_cast<uint8_t*>(m_data.address()) + 32;
    }

    ~Buffer2()
    {
    }

    void acquire()
    {
        m_referenceCount += 1;
    }

    int referenceCount() const
    {
        return m_referenceCount;
    }

    void release()
    {
        m_referenceCount -= 1;
        if (!m_referenceCount)
        {
            //destroy();
        }
    }

    const uint8_t* begin() const
    {
        return m_begin;
    }

    const uint8_t* end() const
    {
        return m_end;
    }

    //! Adds data at the end of the buffer.
    //! Copies the given \p data at the end of the buffer.
    template <typename TType>
    void push_back(const TType& data)
    {
        assert(m_end + sizeof(data)
               <= static_cast<uint8_t*>(m_data.address()) + BUFFER_SIZE);
        std::memcpy(m_end, &data, sizeof(data));
        m_end += sizeof(data);
    }

    //! Prepends data to the buffer.
    //! Copies \p data to the start of the buffer.
    template <typename TType>
    void push_front(const TType& data)
    {
        m_begin -= sizeof(data);
        assert(m_begin >= static_cast<uint8_t*>(m_data.address()));
        std::memcpy(m_begin, &data, sizeof(data));
    }

    //! Returns the size of the buffer.
    std::size_t size() const
    {
        return static_cast<std::size_t>(m_end - m_begin);
    }

private:
    //! The storage of the buffer.
    ::boost::aligned_storage<BUFFER_SIZE>::type m_data;
    //! Points to the first valid byte in m_data.
    uint8_t* m_begin;
    //! Points just next to the last valid byte in m_data.
    uint8_t* m_end;
    //! A pointer to the pool from which the buffer was allocated.
    void* m_pool;

    //! \todo: Must be atomic
    int m_referenceCount;
};

class BufferHandle
{
public:
    BufferHandle(Buffer2* buffer);
    ~BufferHandle();

private:
    Buffer2* m_buffer;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        slist_hook_t;
    slist_hook_t m_slistHook;
};

typedef boost::intrusive::slist<
        BufferHandle,
        boost::intrusive::member_hook<
            BufferHandle,
            BufferHandle::slist_hook_t,
            &BufferHandle::m_slistHook>,
        boost::intrusive::cache_last<true> > Buffer2Queue;

#endif // BUFFER2_HPP
