#ifndef UNET_REFCOUNTED_HPP
#define UNET_REFCOUNTED_HPP

#include <OperatingSystem/OperatingSystem.h>

namespace uNet
{

//! A poor mans reference counter.
//!
//! \todo This should actually be a std::atomic<unsigned> and use atomic
//! memory accesses rather than blocking a mutex whenever the reference
//! count is changed.
struct ReferenceCounter
{
    ReferenceCounter()
        : m_value(0)
    {
    }

    void inc()
    {
        OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
        ++m_value;
    }

    bool dec()
    {
        OperatingSystem::lock_guard<OperatingSystem::mutex> lock(m_mutex);
        --m_value;
        return m_value > 0;
    }

    OperatingSystem::mutex m_mutex;
    unsigned m_value;
};

//! A pointer to a reference counted object.
//!
//! Requires the following 3 free functions:
//! void acquire(TType*);
//! bool release(TType*);
//! void dispose(TType*);
//! With \p acquire() the reference counter is increased by one. The
//! \p release() function decreases it by one and returns \p true if the
//! reference counter is non-zero. Finally, \p dispose deletes the object.
//! All 3 functions must cope with null-pointers.
template <typename TType>
class refcounted_ptr
{
    // Tips for the implementation:
    // http://www.boost.org/community/counted_body.html
    // http://www.parashift.com/c++-faq/ref-count-simple.html
    // http://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr

public:
    BOOST_CONSTEXPR refcounted_ptr()
        : m_ptr(0)
    {
    }

    explicit refcounted_ptr(TType* ptr)
        : m_ptr(ptr)
    {
        acquire(m_ptr);
    }

    refcounted_ptr(const refcounted_ptr& other)
        : m_ptr(other.m_ptr)
    {
        acquire(m_ptr);
    }

    ~refcounted_ptr()
    {
        reset();
    }

    refcounted_ptr& operator= (const refcounted_ptr& other)
    {
        reset(other.m_ptr);
        return *this;
    }

    TType* get() const
    {
        return m_ptr;
    }

    void reset()
    {
        reset(0);
    }

    void reset(TType* ptr)
    {
        // Copy the new ptr before releasing the old one. This even works with
        // self-assignment.
        acquire(ptr);
        TType* old = m_ptr;
        m_ptr = ptr;

        if (!release(old))
        {
            dispose(old);
        }
    }

    TType& operator* () const
    {
        return *m_ptr;
    }

    TType* operator-> () const
    {
        return m_ptr;
    }

    operator bool () const
    {
        return m_ptr != 0;
    }

private:
    //! A pointer to the reference counted object.
    TType* m_ptr;
};

} // namespace uNet

#endif // UNET_REFCOUNTED_HPP
