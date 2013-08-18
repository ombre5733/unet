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

#include "chrono.hpp"

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

namespace detail
{

template <typename DerivedT>
class generic_mutex : boost::noncopyable
{
public:
    // Create a generic mutex.
    generic_mutex()
        : m_id(0)
    {
        osMutexDef_t mutexDef = { m_cmsisMutexControlBlock };
        m_id = osMutexCreate(&mutexDef);
        if (m_id == 0)
            ::boost::throw_exception(std::system_error());
    }

    // Destroys the mutex.
    ~generic_mutex()
    {
        if (m_id)
            osMutexDelete(m_id);
    }

    // Locks the mutex. Calls post_lock_check() after a successful lock.
    void lock()
    {
        osStatus status = osMutexWait(m_id, osWaitForever);
        if (status != osOK)
            ::boost::throw_exception(std::system_error());
        derived()->post_lock_check();
    }

    // Tries to lock the mutex. If successful, returns the result of calling
    // post_try_lock_correction().
    bool try_lock()
    {
        osStatus status = osMutexWait(m_id, 0);
        if (status == osOK)
        {
            return derived()->post_try_lock_correction();
        }
        else
            return false;
    }

    void unlock()
    {
        osStatus status = osMutexRelease(m_id);
        // Just check the return code but do not throw because unlock is
        // called from the destructor of lock_guard, for example.
        //! \todo I think, we can throw exceptions, too.
        BOOST_ASSERT(status == osOK);
    }

protected:
    uint32_t m_cmsisMutexControlBlock[3];
    osMutexId m_id;

    P_MUCB mutexControlBlock()
    {
        return (P_MUCB)m_cmsisMutexControlBlock;
    }

    DerivedT* derived()
    {
        return static_cast<DerivedT*>(this);
    }

    void post_lock_check()
    {
    }

    bool post_try_lock_correction()
    {
        return true;
    }
};

class mutex_try_locker
{
public:
    mutex_try_locker(osMutexId id)
        : m_id(id)
    {
    }

    bool operator() (int32_t timeout)
    {
        osStatus status = osMutexWait(m_id, timeout);
        if (status == osOK)
            return true;

        if (   status != osErrorTimeoutResource
            && status != osErrorResource)
            throw -1;

        return false;
    }

private:
    osMutexId m_id;
};

template <typename DerivedT>
class generic_timed_mutex : public generic_mutex<DerivedT>
{
public:
    template <typename RepT, typename PeriodT>
    bool try_lock_for(const chrono::duration<RepT, PeriodT>& d)
    {
        mutex_try_locker locker(m_id);
        return chrono::detail::cmsis_wait<RepT, PeriodT, mutex_try_locker>(
                    d, locker);
    }
};

template <typename BaseT>
class nonrecursive_adapter : public BaseT<nonrecursive_adapter>
{
protected:
    void post_lock_check()
    {
        assert(pmucb()->level == 1);
    }

    bool post_try_lock_correction()
    {
        if (pmucb()->level == 1)
            return true;

        assert(mutexControlBlock()->level == 2);
        osStatus status = osMutexRelease(m_id);
        assert(status == OS_OK);
        return false;
    }
};

} // namespace detail

class mutex
#ifndef OSL_DOXYGEN_RUN
        : public detail::nonrecursive_adapter<detail::generic_mutex>
#endif // OSL_DOXYGEN_RUN
{
public:
#ifdef OSL_DOXYGEN_RUN
    //! Creates a mutex.
    mutex();
    //! Destroys the mutex.
    ~mutex();
    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //! It is undefined behaviour, if the calling thread has already acquired
    //! the mutex and wants to lock it again.
    //!
    //! \sa try_lock()
    void lock();
    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns \p false without blocking.
    bool try_lock();
    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock();
#endif // OSL_DOXYGEN_RUN
};

class timed_mutex
#ifndef OSL_DOXYGEN_RUN
        : public detail::nonrecursive_adapter<detail::generic_timed_mutex>
#endif // OSL_DOXYGEN_RUN
{
public:
#ifdef OSL_DOXYGEN_RUN
    //! Creates a mutex with support for timeout.
    timed_mutex();
    //! Destroys the mutex.
    ~timed_mutex();
    //! Locks the mutex.
    //! Blocks the current thread until this mutex has been locked by it.
    //! It is undefined behaviour, if the calling thread has already acquired
    //! the mutex and wants to lock it again.
    //!
    //! \sa try_lock()
    void lock();
    //! Tests and locks the mutex if it is available.
    //! If this mutex is available, it is locked by the calling thread and
    //! \p true is returned. If the mutex is already locked, the method
    //! returns \p false without blocking.
    bool try_lock();
    //! Unlocks the mutex.
    //! Unlocks this mutex which must have been locked previously by the
    //! calling thread.
    void unlock();
#endif // OSL_DOXYGEN_RUN
};

//! A recursive mutex.
class recursive_mutex
#ifndef OSL_DOXYGEN_RUN
        : public detail::generic_mutex<recursive_mutex>
#endif // OSL_DOXYGEN_RUN
{
public:
};

//! A recursive mutex with support for timeout.
class recursive_timed_mutex
#ifndef OSL_DOXYGEN_RUN
        : public detail::generic_timed_mutex<recursive_timed_mutex>
#endif // OSL_DOXYGEN_RUN
{
public:
};

//! A lock guard for RAII-style mutex locking.
template <class MutexT>
class lock_guard : boost::noncopyable
{
public:
    typedef MutexT mutex_type;

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

//! A unique lock for a mutex.
template <class Mutex>
class unique_lock
{
public:
    typedef Mutex mutex_type;

    //! Creates a lock which is not associated with a mutex.
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

    //! Creates a unique lock for a locked mutex.
    //! Creates a unique lock for the given \p mutex. The constructor does
    //! not lock the mutex but assumes that it has already been locked
    //! by the caller.
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
    //! Breaks the association of this lock and its mutex (which is returned
    //! by this function). The lock won't interact with the mutex any longer
    //! (it won't even unlock the mutex). Instead the responsibility is
    //! transfered to the caller.
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
    //! A pointer to the associated mutex.
    mutex_type* m_mutex;
    //! A flag indicating if the mutex has been locked.
    bool m_locked;
};

} // namespace osl

#endif // OSL_CMSIS_MUTEX_HPP
