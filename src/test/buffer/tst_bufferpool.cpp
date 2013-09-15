#include "../bufferpool.hpp"

#include "gtest/gtest.h"

TEST(BufferPool, Constructor)
{
    uNet::BufferPool<256, 1> p;
    ASSERT_FALSE(p.empty());
}

TEST(BufferPool, allocate_and_release)
{
    uNet::BufferPool<256, 1> p;
    for (int i = 0; i < 100; ++i)
    {
        uNet::Buffer* b = p.allocate();
        ASSERT_TRUE(b != 0);
        ASSERT_TRUE(p.empty());
        ASSERT_EQ(&p, b->disposer());

        p.release(b);
        ASSERT_FALSE(p.empty());
    }
}

TEST(BufferPool, dispose)
{
    uNet::BufferPool<256, 1> p;
    for (int i = 0; i < 100; ++i)
    {
        uNet::Buffer* b = p.allocate();
        ASSERT_TRUE(b != 0);
        ASSERT_TRUE(p.empty());
        ASSERT_EQ(&p, b->disposer());

        b->dispose();
        ASSERT_FALSE(p.empty());
    }
}
