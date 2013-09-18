#ifndef UNET_EVENT_HPP
#define UNET_EVENT_HPP

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

class Buffer;
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
        MessageSend
    };

    Event()
        : m_type(Invalid),
          m_interface(0),
          m_next(0)
    {
    }

    Event(const Event& other)
        : m_type(other.m_type),
          m_interface(other.m_interface),
          m_next(0)
    {
        m_data.m_buffer = other.m_data.m_buffer;
    }

    Type type() const
    {
        return m_type;
    }

    //! Creates an event for sending a \p buffer.
    static Event createMessageSendEvent(Buffer* buffer)
    {
        Event ev(MessageSend);
        ev.m_data.m_buffer = buffer;
        return ev;
    }

private:
    explicit Event(Type type)
        : m_type(type),
          m_interface(0),
          m_next(0)
    {
        m_data.m_buffer = 0;
    }

    Type m_type;
    NetworkInterface* m_interface;
    Event* m_next;

    union
    {
        Buffer* m_buffer;
    } m_data;

    template <unsigned>
    friend class EventList;
};

template <unsigned MaxNumEventsT>
class EventList
{
public:
    EventList()
        : m_numEvents(0)
    {
    }

    void enqueue(Event event)
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

    void release(Event* event)
    {
        m_eventPool.destroy(event);
    }

    Event* retrieve()
    {
        m_numEvents.wait();
        OperatingSystem::lock_guard<OperatingSystem::mutex> locker(m_mutex);
        Event* first = m_eventList;
        m_eventList = first->m_next;
        first->m_next = 0;
        return first;
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

} // namespace uNet

#endif // UNET_EVENT_HPP
