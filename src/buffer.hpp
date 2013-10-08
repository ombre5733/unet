#ifndef UNET_BUFFER_HPP
#define UNET_BUFFER_HPP

#include "config.hpp"

#include <boost/intrusive/slist.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

#include <cstdint>
#include <cstring>

// Pool -> TCP -> Kernel -> Ifc -> TCP -> Pool

// Pool -> RPC -> TCP -> Kernel -> Ifc -> TCP -> RPC -> Pool

// Pool -> Udp -> Kernel -> Ifc -> Pool

namespace uNet
{

class BufferBase;

//! A buffer grabber.
//! A BufferGrabber is an object which can take the ownership of a Buffer
//! before it is destructed. The typical use case for a buffer grabber is
//! to implement a protocol which needs to get a buffer back after it has
//! been sent via an interface.
class BufferGrabber
{
public:
    virtual void grab(BufferBase& buffer) = 0;
};

//! A memento for storing some buffer state.
//! The BufferMemento can---to a certain degree---save and restore the state of
//! a Buffer. It does not memorize the buffer's data but only the current
//! iterators. This is sufficient as long as the following processing elements
//! in the chain restrict themselves to prepending or appending data to the
//! buffer but do not touch the data between the iterators.
//!
//! A BufferMemento should be used whenever a processing object
//! wishes to get the buffer back after the processing chain has been finished.
//! As an example, consider a protocol which must be able to re-transmit the
//! data upon request. Before the protocol object \p A submits the buffer to the
//! sending interface, it creates a memento which is added to the buffer. When
//! the interface has sent the data, it pops the most recent memento and returns
//! the buffer back to the object \p A. Then \p A can keep a copy of the buffer
//! until it receives an acknowledge signal from the receiver upon which the
//! buffer will be disposed.
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

    //! Creates a buffer memento.
    //! Creates a buffer memento which will return the buffer to the \p grabber
    //! instead of disposing it.
    //! \todo Take the buffer as argument here?
    explicit BufferMemento(BufferGrabber* grabber)
        : m_grabber(grabber),
          m_begin(0),
          m_end(0)
    {
    }

private:
    //! The object to which the buffer is handed over after it is not needed
    //! in the processing chain any longer.
    BufferGrabber* m_grabber;
    //! Points to the first valid byte in Buffer2::m_data.
    std::uint8_t* m_begin;
    //! Points just past the last valid byte in Buffer2m_data.
    std::uint8_t* m_end;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        memento_list_hook_t;
    memento_list_hook_t m_mementoListHook;

private:
    //! A pointer back to the pool from which this hook was allocated.
    void* m_pool;

    friend class BufferBase;
};

//! A stack of buffer mementos.
typedef boost::intrusive::slist<
        BufferMemento,
        boost::intrusive::member_hook<
            BufferMemento,
            BufferMemento::memento_list_hook_t,
            &BufferMemento::m_mementoListHook>,
        boost::intrusive::cache_last<false> > BufferMementoStack;

//! A buffer processor.
//! A BufferProcessor is an object in the processing chain of a buffer.
//! Every object in the processing stack can add itself to the buffer.
class BufferProcessor
{

};

//! An buffer disposer.
//! This abstract base class specifies the interface of a disposer for an
//! object of type BufferBase. Every buffer holds a pointer to one such object
//! which it can invoke in order to be disposed. Depending on the allocation
//! strategy, the disposer should call delete or return the buffer back to
//! a memory pool.
class BufferDisposer
{
public:
    //! Disposes a buffer.
    //! Disposes the given \p buffer.
    virtual void dispose(BufferBase* buffer) = 0;
};

//! The common base of all buffers.
//! The BufferBase is the common base class of all buffer implementations
//! in the uNet library.
class BufferBase : boost::noncopyable
{
    static const unsigned numReservedBytes = 32;
public:

    explicit BufferBase(std::uint8_t* storageBegin,
                        BufferDisposer* disposer = 0)
        : m_disposer(disposer)
    {
        m_begin = m_end = storageBegin + numReservedBytes;
    }

    //! Adds a memento to the buffer.
    //! Adds the \p memento to this buffer.
    void addMemento(BufferMemento& memento)
    {
        memento.m_begin = m_begin;
        memento.m_end = m_end;
        m_mementoStack.push_front(memento);
    }

    //! Returns the buffer's capacity.
    //! Returns the number of bytes which the buffer can hold.
    std::size_t capacity() const
    {
        return static_cast<std::size_t>(storageEnd() - storageBegin());
    }

    //! Clears the buffer.
    void clear()
    {
        m_begin = m_end = storageBegin() + numReservedBytes;
    }

    //! Returns the buffer disposer.
    BufferDisposer* disposer() const
    {
        return m_disposer;
    }

    //! Disposes the buffer.
    //! If there is still a memento on the memento stack, the buffer is returned
    //! to the top-most memento (the one which has been last recently added).
    //! Otherwise, the buffer is handled over to the disposer.
    void dispose()
    {
        if (!m_mementoStack.empty())
        {
            BufferMemento& memento = m_mementoStack.front();
            m_begin = memento.m_begin;
            m_end = memento.m_end;
            BufferGrabber* grabber = memento.m_grabber;
            m_mementoStack.pop_front();//! \todo should be popfront_and_dispose()
            grabber->grab(*this);
        }
        else if (m_disposer)
            m_disposer->dispose(this);
    }

    //! Returns a pointer to the beginning of the data.
    std::uint8_t* begin()
    {
        return m_begin;
    }

    //! Returns a pointer to the beginning of the data.
    const std::uint8_t* begin() const
    {
        return m_begin;
    }

    void moveBegin(int offset)
    {
        m_begin += offset;
        UNET_ASSERT(m_begin >= storageBegin() && m_begin <= m_end);
    }

    //! Returns a pointer just past the end of the data.
    std::uint8_t* end()
    {
        return m_end;
    }

    //! Returns a pointer just past the end of the data.
    const std::uint8_t* end() const
    {
        return m_end;
    }

    //! Copies an element from the beginning of the buffer.
    //! Copies an element of type \p TType from the beginning of the buffer
    //! without changing the iterators.
    template <typename TType>
    TType copy_front() const
    {
        UNET_ASSERT(m_begin + sizeof(TType) <= m_end);
        TType temp;
        std::memcpy(&temp, m_begin, sizeof(TType));
        return temp;
    }

    //! Pops an element from the beginning of the buffer.
    //! Pops an element of type \p TType from the beginning of the buffer and
    //! advances the start iterator.
    template <typename TType>
    TType pop_front()
    {
        UNET_ASSERT(m_begin + sizeof(TType) <= m_end);
        TType temp;
        std::memcpy(&temp, m_begin, sizeof(TType));
        m_begin += sizeof(TType);
        return temp;
    }

    //! Adds an element at the end of the buffer.
    //! Copies the given \p data element at the end of the buffer.
    template <typename TType>
    void push_back(const TType& data)
    {
        UNET_ASSERT(m_end + sizeof(TType) <= storageEnd());
        std::memcpy(m_end, &data, sizeof(TType));
        m_end += sizeof(TType);
    }

    //! Prepends data to the buffer.
    //! Copies the \p data to the start of the buffer.
    template <typename TType>
    void push_front(const TType& data)
    {
        m_begin -= sizeof(TType);
        UNET_ASSERT(m_begin >= storageBegin());
        std::memcpy(m_begin, &data, sizeof(TType));
    }

    //! Places the begin and end iterators to the beginning of the storage.
    void rewind()
    {
        m_begin = m_end = storageBegin();
    }

    //! Returns the size of the data.
    //! Returns the size of the data in the buffer, i.e. the difference
    //! between the begin() and end() iterators.
    std::size_t size() const
    {
        return static_cast<std::size_t>(m_end - m_begin);
    }

protected:
    //! Returns a pointer to the first byte of the storage.
    virtual std::uint8_t* storageBegin() const = 0;
    //! Returns a pointer just past the last byte of the storage.
    virtual std::uint8_t* storageEnd() const = 0;

private:
    //! Points to the first valid byte in the storage.
    std::uint8_t* m_begin;
    //! Points just past the last valid byte in the storage.
    std::uint8_t* m_end;
    //! The object which is invoked for disposing this buffer.
    BufferDisposer* m_disposer;
    //! A stack for storing the mementos.
    BufferMementoStack m_mementoStack;

public:
    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::normal_link> >
        queue_hook_t;

    //! A hook to put this buffer in a BufferQueue.
    queue_hook_t m_queueHook;
};

//! A buffer queue.
typedef boost::intrusive::slist<
        BufferBase,
        boost::intrusive::member_hook<
            BufferBase,
            BufferBase::queue_hook_t,
            &BufferBase::m_queueHook>,
        boost::intrusive::cache_last<true> > BufferQueue;

//! A concrete buffer template.
//! The Buffer is a concrete implementation of BufferBase. The size of the
//! storage has to be provided at compile time via the template parameter
//! \p TBufferSize and the depth of the processor stack in
//! \p TProcessorStackDepth.
template <unsigned TBufferSize, unsigned TProcessorStackDepth>
class Buffer : public BufferBase
{
public:
    //! Creates a buffer.
    //! Creates a buffer which will be destroyed via the buffer \p disposer.
    //! The disposer may be a null-pointer in which case it is never invoked.
    explicit Buffer(BufferDisposer* disposer = 0)
        : BufferBase(static_cast<std::uint8_t*>(m_data.address()), disposer),
          m_processorStackSize(0)
    {
    }

protected:
    //! \reimp
    virtual std::uint8_t* storageBegin() const
    {
        return static_cast<std::uint8_t*>(m_data.address());
    }

    //! \reimp
    virtual std::uint8_t* storageEnd() const
    {
        return static_cast<std::uint8_t*>(m_data.address()) + TBufferSize;
    }

private:
    //! The storage of the buffer.
    typename boost::aligned_storage<TBufferSize>::type m_data;
    //! The stack of buffer processors.
    BufferProcessor* m_bufferProcessors[TProcessorStackDepth];
    //! The processor stack size.
    std::size_t m_processorStackSize;
};

} // namespace uNet

#endif // UNET_BUFFER_HPP
