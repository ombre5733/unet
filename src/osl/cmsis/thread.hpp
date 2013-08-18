/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#ifndef OSL_CMSIS_THREAD_HPP
#define OSL_CMSIS_THREAD_HPP

#include "chrono.hpp"

#include "cmsis_os.h"

#include <boost/config.hpp>
#include <boost/utility.hpp>

namespace osl
{

class thread : boost::noncopyable //! \todo Or must it be copyable?
{
    //! A representation of a thread identifier.
    //! This class is a wrapper around a thread identifier. It has a small
    //! memory footprint such that copied can be passed around.
    class id
    {
    public:
        id() : m_id(0) {}

        id(osThreadId _id) : m_id(_id) {}

    private:
        friend bool operator== (id lhs, id rhs);
        friend bool operator!= (id lhs, id rhs);
        friend bool operator< (id lhs, id rhs);
        friend bool operator<= (id lhs, id rhs);
        friend bool operator> (id lhs, id rhs);
        friend bool operator>= (id lhs, id rhs);

        osThreadId m_id;
    };

    class attributes
    {
        enum Priority
        {
            Idle = osPriorityIdle,
            Low = osPriorityLow,
            BelowNormal = osPriorityBelowNormal,
            Normal = osPriorityNormal,
            AboveNormal = osPriorityAboveNormal,
            High = osPriorityHigh,
            Realtime = osPriorityRealtime,
            Error = osPriorityError
        };

        Priority priority;
        uint32_t stackSize;
    };

    thread()
    {
    }

    thread(const attributes& attrs)
    {
    }

    thread(void (*fun)(void*), void* arg)
    {
        osThreadDef_t threadDef = { fun, DEFAULT_PRIORITY, 0, 0 };
        m_id = osThreadCreate(&threadDef, arg);
    }

    ~thread()
    {
        //! \todo Check if the thread is still running. If so, call
        //! std::terminate(). The C++11 standard does not allow to invoke
        //! the destructor when the thread is joinable()
    }

    //! Returns the id of the thread.
    id get_id() const BOOST_NOEXCEPT
    {
        return m_id;
    }

    //! Returns the number of threads which can run concurrently on this
    //! hardware.
    static unsigned hardware_concurrency() BOOST_NOEXCEPT
    {
        return 1;
    }

private:
    id m_id;
};

//! Compares two thread ids for equality.
//! Returns \p true, if \p lhs and \p rhs are equal.
inline
bool operator== (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id == rhs.m_id;
}

//! Compares two thread ids for inequality.
//! Returns \p true, if \p lhs and \p rhs are not equal.
inline
bool operator!= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id != rhs.m_id;
}

//! Less-than comparison for thread ids.
//! Returns \p true, if \p lhs is less than \p rhs.
inline
bool operator< (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id < rhs.m_id;
}

//! Less-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is less than or equal to \p rhs.
inline
bool operator<= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id <= rhs.m_id;
}

//! Greater-than comparison for thread ids.
//! Returns \p true, if \p lhs is greater than \p rhs.
inline
bool operator> (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id > rhs.m_id;
}

//! Greater-than or equal comparison for thread ids.
//! Returns \p true, if \p lhs is greater than or equal to \p rhs.
inline
bool operator>= (thread::id lhs, thread::id rhs) BOOST_NOEXCEPT
{
    return lhs.m_id >= rhs.m_id;
}

namespace this_thread
{

//! Returns the id of the current thread.
inline
osl::thread::id get_id()
{
    return osl::thread::id(osThreadGetId());
}

namespace detail
{

class thread_sleeper
{
public:
    bool operator() (int32_t timeout)
    {
        status = osDelay(timeout);
        OSL_ASSERT(status == osEventTimeout);
    }
};

} // namespace detail

//! Puts the current thread to sleep.
//! Blocks the execution of the current thread for the given duration \p d.
template <typename RepT, typename PeriodT>
void sleep_for(const chrono::duration<RepT, PeriodT>& d) BOOST_NOEXCEPT
{
    detail::thread_sleeper sleeper;
    chrono::detail::cmsis_wait<RepT, PeriodT, detail::thread_sleeper>(
                d, sleeper);
}

template <typename ClockT, typename DurationT>
void sleep_until(const chrono::time_point<ClockT, DurationT>& timePoint) BOOST_NOEXCEPT;

//! Triggers a rescheduling of the executing threads.
inline
void yield()
{
    osStatus status = osThreadYield();
    OSL_ASSERT(status == osOK);
    OSL_UNUSED(status);
}

} // namespace this_thread

} // namespace osl

#endif // OSL_CMSIS_THREAD_HPP
