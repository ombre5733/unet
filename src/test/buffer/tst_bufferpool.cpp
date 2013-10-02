#include "../bufferpool.hpp"

#include "gtest/gtest.h"

TEST(BufferPool, Constructor)
{
    typedef uNet::BufferPool<256, 1> pool_t;
    pool_t p;
    ASSERT_FALSE(p.empty());
}

TEST(BufferPool, allocate_and_release)
{
    typedef uNet::BufferPool<256, 1> pool_t;
    pool_t p;
    for (int i = 0; i < 100; ++i)
    {
        pool_t::buffer_type* b = p.allocate();
        ASSERT_TRUE(b != 0);
        ASSERT_TRUE(p.empty());
        ASSERT_EQ(&p, b->disposer());

        p.release(b);
        ASSERT_FALSE(p.empty());
    }
}

TEST(BufferPool, allocate_and_dispose)
{
    typedef uNet::BufferPool<256, 1> pool_t;
    pool_t p;
    for (int i = 0; i < 100; ++i)
    {
        pool_t::buffer_type* b = p.allocate();
        ASSERT_TRUE(b != 0);
        ASSERT_TRUE(p.empty());
        ASSERT_EQ(&p, b->disposer());

        b->dispose();
        ASSERT_FALSE(p.empty());
    }
}
