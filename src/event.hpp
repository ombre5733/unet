#ifndef UNET_EVENT_HPP
#define UNET_EVENT_HPP

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

class Buffer;
class NetworkInterface;

//! A kernel event.
//! The Event is a happening in time inside the kernel or its associated
//! components.
class Event
{
public:
    //! An enumeration of event types.
    enum Type
    {
        LinkConnection,
        LinkConnectionLoss,
        MessageReceive,
        MessageSend
    };

    Event() {}

    //! Creates an event to send a \p buffer.
    static Event createMessageSendEvent(Buffer* buffer)
    {
        Event ev(MessageSend);
        ev.m_data.m_buffer = buffer;
        return ev;
    }

private:
    explicit Event(Type type)
        : m_type(type),
          m_interface(0)
    {
        m_data.m_buffer = 0;
    }

    Type m_type;
    NetworkInterface* m_interface;

    union
    {
        Buffer* m_buffer;
    } m_data;
};

template <unsigned MaxNumEventsT>
class EventList
{
public:
    EventList()
        : m_numEvents(0)
    {
    }

    void push(Event ev)
    {
        OperatingSystem::lock_guard<OperatingSystem::mutex> locker(m_mutex);
        if (m_numEvents < MaxNumEventsT)
            m_events[m_numEvents++] = ev;
    }

private:
    OperatingSystem::mutex m_mutex;
    Event m_events[MaxNumEventsT];
    unsigned m_numEvents;
};

} // namespace uNet

#endif // UNET_EVENT_HPP
