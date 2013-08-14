#ifndef BUFFER2_HPP
#define BUFFER2_HPP

#include <boost/intrusive/slist.hpp>
#include <boost/type_traits.hpp>

#include <cstdint>
#include <cstring>

// Pool -> TCP -> Kernel -> Ifc -> TCP -> Pool

// Pool -> RPC -> TCP -> Kernel -> Ifc -> TCP -> RPC -> Pool

// Pool -> Udp -> Kernel -> Ifc -> Pool

class Buffer2;

//! A buffer grabber.
//! A BufferGrabber is an object which can take the ownership of a Buffer
//! before it is destructed. The typical use case for a buffer grabber is
//! to implement a protocol which needs to get a buffer back after it has
//! been sent via an interface.
class BufferGrabber
{
public:
    virtual void grab(Buffer2& buffer) = 0;
};

//! A memento for storing some buffer state.
//! The BufferMemento can---to a certain degree---save and restore the state of
//! a Buffer. It does not copy the buffer's data but only the current iterators.
//! This is sufficient as long as the following processing chain restricts
//! itself to prepending or appending data to the buffer and does not modify the
//! elements between the iterators.
//!
//! A BufferMemento should be used whenever a processing object
//! wishes to get the buffer back after the processing chain has been finished.
//! As an example, consider a protocol which must be able to re-transmit the
//! data upon request. Before the protocol submits the buffer to the sending
//! interface, it creates a memento which is added to the buffer. When the
//! interface has sent the data, it pops the most recent memento and returns
//! the buffer back to the protocol. The protocol can backup the buffer in some
//! list and keep it until it receives an acknowledge from the receiver.
class BufferMemento
{
public:
    //! A helper class for disposing a BufferMemento.
    class Disposer
    {
        void operator() (BufferMemento* memento)
        {
            //memento->m_pool.destroy(memento);
        }
    };

    explicit BufferMemento(BufferGrabber* grabber)
        : m_grabber(grabber),
          m_begin(0),
          m_end(0)
    {
    }

private:
    BufferGrabber* m_grabber;
    //! Points to the first valid byte in m_data.
    uint8_t* m_begin;
    //! Points just past the last valid byte in m_data.
    uint8_t* m_end;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        memento_list_hook_t;
    memento_list_hook_t m_mementoListHook;

private:
    //! A pointer back to the pool from which this hook was allocated.
    void* m_pool;

    friend class Buffer2;
};

//! A stack of buffer mementos.
typedef boost::intrusive::slist<
        BufferMemento,
        boost::intrusive::member_hook<
            BufferMemento,
            BufferMemento::memento_list_hook_t,
            &BufferMemento::m_mementoListHook>,
        boost::intrusive::cache_last<false> > BufferMementoStack;

//! An buffer disposer.
//! This abstract base class specifies the interface of a disposer for an
//! object of type Buffer. Every buffer holds a pointer to one such object
//! which it can invoke in order to be disposed. Depending on the allocation
//! strategy, the disposer should call delete or return the buffer back to
//! a memory pool.
class BufferDisposer
{
public:
    virtual void operator() (Buffer2* buffer) = 0;
};

class Buffer2
{
public:
    static const int BUFFER_SIZE = 256;

    explicit Buffer2(BufferDisposer* disposer)
        : m_disposer(disposer)
    {
        m_begin = m_end = static_cast<uint8_t*>(m_data.address()) + 32;
    }

    ~Buffer2()
    {
    }

    void addMemento(BufferMemento& memento)
    {
        memento.m_begin = m_begin;
        memento.m_end = m_end;
        m_mementoStack.push_front(memento);
    }

    void dispose()
    {
        if (!m_mementoStack.empty())
        {
            BufferMemento& memento = m_mementoStack.front();
            memento.m_grabber->grab(*this);
            //! \todo m_mementoStack.pop_front_and_dispose();
        }
        else
            m_disposer->operator ()(this);
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
    //! Points just past the last valid byte in m_data.
    uint8_t* m_end;
    //! The object which is invoked for disposing this buffer.
    BufferDisposer* m_disposer;
    //! A stack for storing the mementos.
    BufferMementoStack m_mementoStack;
};

#endif // BUFFER2_HPP
