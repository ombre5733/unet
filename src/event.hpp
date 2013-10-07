#ifndef UNET_EVENT_HPP
#define UNET_EVENT_HPP

#include "config.hpp"

#include "networkaddress.hpp"

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{
class BufferBase;
class NetworkInterface;

//! A kernel event.
//! The Event is a happening in time inside the kernel or its associated
//! components. The kernel keeps a list of events which have to be processed.
class Event
{
public:
    //! An enumeration of event types.
    enum Type
    {
        Invalid,
        LinkConnection,
        LinkConnectionLoss,
        MessageReceive,
        MessageSend,

        SendLinkLocalBroadcast,
        SendRawMessage,

        StopKernel
    };

    Event()
        : m_type(Invalid),
          m_next(0)
    {
    }

    Event(const Event& other)
        : m_type(other.m_type),
          m_next(0)
    {
        m_interface = other.m_interface;
        m_buffer = other.m_buffer;
    }

    HostAddress hostAddress() const
    {
        return m_hostAddress;
    }

    Type type() const
    {
        return m_type;
    }

    NetworkInterface* networkInterface() const
    {
        return m_interface;
    }

    BufferBase* buffer() const
    {
        return m_buffer;
    }

    //! Creates a message receive event.
    //! Creates an event to signal the reception of a \p buffer.
    static Event createMessageReceiveEvent(NetworkInterface* ifc,
                                           BufferBase* buffer)
    {
        Event ev(MessageReceive);
        ev.m_interface = ifc;
        ev.m_buffer = buffer;
        return ev;
    }

    //! Creates an event for sending a raw message.
    //! Creates an event for sending the given \p buffer without prepending
    //! a network header.
    static Event createSendRawMessageEvent(BufferBase* buffer)
    {
        Event ev(SendRawMessage);
        ev.m_buffer = buffer;
        return ev;
    }

    static Event createSendLinkLocalBroadcast(NetworkInterface* ifc,
                                              BufferBase* buffer)
    {
        Event ev(SendLinkLocalBroadcast);
        ev.m_interface = ifc;
        ev.m_buffer = buffer;
        return ev;
    }

    //! Creates a message send event.
    //! Creates an event for sending a \p buffer.
    static Event createMessageSendEvent(BufferBase* buffer)
    {
        Event ev(MessageSend);
        ev.m_buffer = buffer;
        return ev;
    }

    //! \todo Remove this method again and allow the kernel to create events
    //! with arbitrary type.
    static Event createStopKernelEvent()
    {
        Event ev(StopKernel);
        return ev;
    }

private:
    explicit Event(Type type)
        : m_type(type),
          m_next(0)
    {
        m_interface = 0;
        m_buffer = 0;
    }

    Type m_type;
    Event* m_next;

    NetworkInterface* m_interface;
    HostAddress m_hostAddress;
    BufferBase* m_buffer;

    template <unsigned>
    friend class EventList;
};

//! An event list.
//! The EventList is a list of events.
template <unsigned MaxNumEventsT>
class EventList
{
public:
    EventList()
        : m_eventList(0),
          m_numEvents(0)
    {
    }

    //! Adds an event.
    //! Adds the \p event to the list.
    void enqueue(const Event& event)
    {
        Event* ev = m_eventPool.construct(event);

        OperatingSystem::lock_guard<OperatingSystem::mutex> locker(m_mutex);
        if (!m_eventList)
            m_eventList = ev;
        else
        {
            Event* iter = m_eventList;
            while (iter->m_next)
                iter = iter->m_next;
            iter->m_next = ev;
        }
        m_numEvents.post();
    }

    Event retrieve()
    {
        m_numEvents.wait();
        OperatingSystem::lock_guard<OperatingSystem::mutex> locker(m_mutex);
        Event* first = m_eventList;
        m_eventList = first->m_next;
        first->m_next = 0;

        Event temp = *first;
        m_eventPool.destroy(first);
        return temp;
    }

private:
    //! A mutex to synchronize accesses to the object.
    OperatingSystem::mutex m_mutex;
    //! \todo Change this to a list sorted by timeout.
    Event* m_eventList;
    //! A pool for allocating events.
    OperatingSystem::counting_object_pool<Event, MaxNumEventsT> m_eventPool;
    //! The number of events which have been enqueued in the list.
    OperatingSystem::semaphore m_numEvents;
};

//! A helper to release an event from an event list.
//! The EventReleaser releases an event from an event list in its destructor.
//! A typical use case is
//! \code
//! EventList list;
//! Event* event = list.retrieve();
//!
//! // Remove the event from the list when we have dispatched it.
//! EventReleaser releaser(list, *event);
//! if (event->type() == ...)
//! {
//!    // Handle this event type.
//! }
//! else if (event->type() == ...)
//! {
//!    // Handle another event type.
//! }
//! \endcode
template <typename ElementListT>
class EventReleaser
{
public:
    //! Creates an event releaser.
    EventReleaser(ElementListT& list, Event& event)
        : m_eventList(list),
          m_event(event)
    {
    }

    ~EventReleaser()
    {
        m_eventList.release(&m_event);
    }

private:
    ElementListT& m_eventList;
    Event& m_event;
};

} // namespace uNet

#endif // UNET_EVENT_HPP
