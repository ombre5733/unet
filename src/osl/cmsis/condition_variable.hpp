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

#ifndef OSL_CMSIS_CONDITIONVARIABLE_HPP
#define OSL_CMSIS_CONDITIONVARIABLE_HPP

#include "chrono.hpp"
#include "mutex.hpp"
#include "semaphore.hpp"

#include <boost/utility.hpp>

namespace osl
{

namespace detail
{

template <typename LockT>
class lock_releaser
{
public:
    typedef LockT lock_type;

    explicit lock_releaser(lock_type& lock)
        : m_lock(lock)
    {
        m_lock.unlock();
    }

    ~lock_releaser()
    {
        m_lock.lock();
    }

private:
    lock_type& m_lock;
};

} // namespace detail

class condition_variable : boost::noncopyable
{
public:
    condition_variable()
        : m_waiters(0)
    {
    }

    ~condition_variable()
    {
        assert(m_waiters == 0);
    }

    void notify_one() BOOST_NOEXCEPT
    {
        lock_guard<recursive_timed_mutex> locker(m_mutex);

        Waiter* head = m_waiters;
        if (head != 0)
        {
            m_waiters = head->next;
            head->dequeued = true;
            head->signal.release();
        }
    }

    void notify_all() BOOST_NOEXCEPT
    {
        lock_guard<recursive_timed_mutex> locker(m_mutex);

        for (Waiter* head = m_waiters; head != 0; head = head->next)
        {
            head->dequeued = true;
            head->signal.release();
        }
    }

    void wait(unique_lock<mutex>& lock)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        {
            lock_guard<recursive_timed_mutex> locker(m_mutex);

            // TODO: enqueue using priorities and change to a FIFO
            w.next = m_waiters;
            m_waiters = &w;
        }

        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait();
        }
    }

    template <typename RepT, typename PeriodT>
    void wait_for(unique_lock<recursive_timed_mutex>& lock,
                  const chrono::duration<RepT, PeriodT>& timeout)
    {
        // First enqueue ourselfs in the list of waiters.
        Waiter w;
        {
            lock_guard<recursive_timed_mutex> locker(m_mutex);

            // TODO: enqueue using priorities and change to a FIFO
            w.next = m_waiters;
            m_waiters = &w;
        }

        // We can only unlock the lock when we are sure that a signal will
        // reach our thread.
        {
            detail::lock_releaser releaser(lock);
            // Wait until we receive a signal, then re-lock the lock.
            w.signal.wait_for(timeout);
            if (timedout)
            {
                lock_guard<recursive_timed_mutex> locker(m_mutex);
                if (!w.dequeued)
                {
                    remove_w_from_queue();
                }
            }
        }
    }

private:
    struct Waiter
    {
        Waiter()
            : next(0),
              dequeued(false)
        {
        }

        Waiter* next;
        semaphore signal;
        bool dequeued;
    };

    recursive_timed_mutex m_mutex;
    Waiter* m_waiters;
};

} // namespace osl

#endif // OSL_CMSIS_CONDITIONVARIABLE_HPP
