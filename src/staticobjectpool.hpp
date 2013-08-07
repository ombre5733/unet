#ifndef STATICOBJECTPOOL_HPP
#define STATICOBJECTPOOL_HPP

// boost::math::static_lcm
#include <boost/math/common_factor_ct.hpp>
// boost::simple_segregated_storage
//#include <boost/pool/simple_segregated_storage.hpp>
#include "simple_segregated_storage.hpp"
// boost::alignment_of
#include <boost/type_traits/alignment_of.hpp>
// boost::aligned_storage
#include <boost/type_traits/aligned_storage.hpp>

template <typename TType, unsigned TNumElem>
class StaticObjectPool : protected boost::simple_segregated_storage<std::size_t>
{
public:
    typedef TType element_type;

private:
    typedef std::size_t size_type;

    // The chunk size has to be a multiple of sizeof(void*) and big enough
    // to hold an element.
    BOOST_STATIC_CONSTANT(size_type, chunk_size =
        (::boost::math::static_lcm<sizeof(void*),
                                   sizeof(element_type)>::value));
    // The memory block must be able to hold TNumElem elements.
    BOOST_STATIC_CONSTANT(size_type, block_size =
        (chunk_size * TNumElem));
    // For the simple_segregated_storage we need to have a chunk aligned such
    // that it can be cast to a void*. When returning a chunk-pointer, to
    // the user, the chunk needs to have the alignment of the element_type.
    BOOST_STATIC_CONSTANT(size_type, min_align =
        (::boost::math::static_lcm< ::boost::alignment_of<void*>::value,
                                    ::boost::alignment_of<element_type>::value>::value));

    typedef boost::simple_segregated_storage<std::size_t> storage_t;
    storage_t& storage() { return *this; }
    const storage_t& storage() const { return *this; }

public:
    StaticObjectPool()
    {
        // simple_segregated_storage requires that
        // - chunk_size >= sizeof(void*)
        // - chunk_size = sizeof(void*) * i, for some integer i
        // - block_size >= chunk_size
        // - Block is properly aligned for an array of object of
        //   size chunk_size and array of void*
        storage().add_block(&m_data, sizeof(m_data), chunk_size);
    }

    bool empty() const
    {
        return storage().empty();
    }

    TType* malloc()
    {
        if (empty())
            return 0;
        else
            return static_cast<element_type*>(storage().malloc());
    }

    void free(element_type* const element)
    {
        storage().free(element);
    }

private:
    typename ::boost::aligned_storage<block_size, min_align>::type m_data;
};

#endif // STATICOBJECTPOOL_HPP
