#include "../../staticobjectpool.hpp"

#include <boost/type_traits.hpp>

#include "gtest/gtest.h"

static inline bool is_aligned(const void* pointer, size_t byte_count)
{
    return (uintptr_t)pointer % byte_count == 0;
}

struct ComplexDouble
{
    double re;
    double im;
};

class NonCopyable
{
public:
    NonCopyable(int i) {}

private:
    NonCopyable();
    NonCopyable(const NonCopyable&);
    NonCopyable& operator= (const NonCopyable&);
};


template <typename T>
class StaticObjectPoolTest : public ::testing::Test
{
};

typedef ::testing::Types<char, int, unsigned int,
                         ComplexDouble,
                         NonCopyable> TypesToTest;
TYPED_TEST_CASE(StaticObjectPoolTest, TypesToTest);

TYPED_TEST(StaticObjectPoolTest, Initialization)
{
    StaticObjectPool<TypeParam, 1> pool;
    ASSERT_EQ(false, pool.empty());
}

TYPED_TEST(StaticObjectPoolTest, Malloc)
{
    const unsigned NUM_ELEMS = 128;

    StaticObjectPool<TypeParam, NUM_ELEMS> pool;
    std::vector<TypeParam*> elements;

    for (unsigned i = 0; i < NUM_ELEMS; ++i)
    {
        ASSERT_EQ(false, pool.empty());
        elements.push_back(pool.malloc());
        ASSERT_EQ(true, elements.back() != 0);
    }

    for (unsigned i = 0; i < 10; ++i)
    {
        ASSERT_EQ(true, pool.empty());
        ASSERT_EQ(true, pool.malloc() == 0);
    }

    while (!elements.empty())
    {
        pool.free(elements.back());
        elements.pop_back();
        ASSERT_EQ(false, pool.empty());
    }
}

TYPED_TEST(StaticObjectPoolTest, Alignment)
{
    const unsigned NUM_ELEMS = 128;

    StaticObjectPool<TypeParam, NUM_ELEMS> pool;
    std::vector<TypeParam*> elements;

    for (unsigned i = 0; i < NUM_ELEMS; ++i)
    {
        elements.push_back(pool.malloc());
        ASSERT_EQ(true, elements.back() != 0);
        ASSERT_EQ(true, is_aligned(elements.back(),
                                   boost::alignment_of<TypeParam>::value));
    }
}
