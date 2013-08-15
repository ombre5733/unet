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

#ifndef OSL_CMSIS_MUTEX_HPP
#define OSL_CMSIS_MUTEX_HPP

#include "cmsis_os.h"

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/throw_exception.hpp>
#include <boost/utility.hpp>

namespace osl
{

struct defer_lock_t {};
struct try_to_lock_t {};
struct adopt_lock_t {};

BOOST_CONSTEXPR_OR_CONST defer_lock_t defer_lock = defer_lock_t();
BOOST_CONSTEXPR_OR_CONST defer_lock_t try_to_lock = try_to_lock_t();
BOOST_CONSTEXPR_OR_CONST defer_lock_t adopt_lock = adopt_lock_t();

//! A recursive mutex with support for timeout.
class recursive_timed_mutex : boost::noncopyable
{
public:
    //! Creates a recursive mutex with support for timeout.
    recursive_timed_mutex()
        : m_id(0)
    {
        osMutexDef_t mutexDef = { m_cmsisMutexControlBlock };
        m_id = osMutexCreate(&mutexDef);
        if (m_id == 0)
            ::boost::throw_exception(std::system_error());
    }

    ~recursive_timed_mutex()
    {
        if (m_id)
            osMutexDelete(m_id);
    }

    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //!
    //! \sa try_lock()
    void lock()
    {
        osStatus status = osMutexWait(m_id, osWaitForever);
        if (status != osOK)
            ::boost::throw_exception(std::system_error());
    }

    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns \p false without blocking.
    bool try_lock()
    {
        osStatus status = osMutexWait(m_id, 0);
        if (status == osOK)
            return true;
        else
            return false;
    }

    //bool try_lock_for(duration);

    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock()
    {
        osStatus status = osMutexRelease(m_id);
        // Just check the return code but do not throw because unlock is
        // called from the destructor of lock_guard, for example.
        //! \todo I think, we can throw exceptions, too.
        BOOST_ASSERT(status == osOK);
    }

private:
    uint32_t m_cmsisMutexControlBlock[3];
    osMutexId m_id;
};

//! A lock guard for RAII-style mutex locking.
template <class Mutex>
class lock_guard : boost::noncopyable
{
public:
    typedef Mutex mutex_type;

    //! Creates a lock guard.
    //! Creates a lock guard and locks the given \p mutex.
    explicit lock_guard(mutex_type& mutex)
        : m_mutex(mutex)
    {
        m_mutex.lock();
    }

    //! Creates a lock guard which adopts a lock.
    //! Creates a lock guard for a \p mutex but does not lock the mutex. Instead
    //! the calling thread must have locked the mutex before creating the
    //! guard.
    //! The guard will still unlock the mutex when it goes out of scope.
    lock_guard(mutex_type& mutex, adopt_lock_t /*tag*/)
        : m_mutex(mutex)
    {
    }

    //! Destroys the lock guard.
    //! Destroys the lock guard and thereby unlocks the guarded mutex.
    ~lock_guard()
    {
        m_mutex.unlock();
    }

private:
    //! The mutex which is guarded.
    mutex_type& m_mutex;
};

template <class Mutex>
class unique_lock
{
public:
    typedef Mutex mutex_type;

    unique_lock() BOOST_NOEXCEPT
        : m_mutex(0),
          m_locked(false)
    {
    }

    //! Creates a unique lock with locking.
    //! Creates a unique lock tied to the \p mutex and locks it.
    explicit unique_lock(mutex_type& mutex)
        : m_mutex(&mutex),
          m_locked(false)
    {
        mutex.lock();
        m_locked = true;
    }

    //! Creates a unique lock without locking.
    //! Creates a unique lock which will be tied to the given \p mutex but
    //! does not lock this mutex.
    unique_lock(mutex_type& mutex, defer_lock_t /*tag*/) BOOST_NOEXCEPT
        : m_mutex(&mutex),
          m_locked(false)
    {
    }

    unique_lock(mutex_type& mutex, try_to_lock_t /*tag*/)
        : m_mutex(&mutex),
          m_locked(false)
    {
        m_locked = mutex.try_lock();
    }

    unique_lock(mutex_type& mutex, adopt_lock_t /*tag*/)
        : m_mutex(&mutex),
          m_locked(true)
    {
    }

    //! \todo Timed constructors are missing

    //! Destroys the unique lock.
    //! If the lock has an associated mutex and has locked this mutex, the
    //! mutex is unlocked.
    ~unique_lock()
    {
        if (m_mutex && m_locked)
            m_mutex.unlock();
    }

    //! Returns a pointer to the associated mutex.
    //! Returns a pointer to the mutex to which this lock is tied. This may
    //! be a null-pointer, if no mutex has been supplied so far.
    mutex_type* mutex() const BOOST_NOEXCEPT
    {
        return m_mutex;
    }

    //! Checks if this lock owns a locked mutex.
    //! Returns \p true, if a mutex is tied to this lock and the lock has
    //! ownership of it.
    bool owns_lock() const
    {
        return m_mutex && m_locked;
    }

    //! Releases the mutex without unlocking.
    //! Releases this lock from its associated mutex. The mutex is not unlocked
    //! but the caller is responsible for unlocking it. The method returns
    //! a pointer to the associated mutex.
    mutex_type* release() BOOST_NOEXCEPT
    {
        mutex_type* m = m_mutex;
        m_mutex = 0;
        m_locked = false;
        return m;
    }

    //! Unlocks the associated mutex.
    bool unlock()
    {
        if (!m_mutex || !m_locked)
            ::boost::throw_exception(std::system_error);
        m_mutex->unlock();
        m_locked = false;
    }

private:
    mutex_type* m_mutex;
    bool m_locked;
};

} // namespace osl

#endif // OSL_CMSIS_MUTEX_HPP
