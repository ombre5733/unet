#include "../bufferpool.hpp"

#include "gtest/gtest.h"

TEST(BufferPool, Constructor)
{
    uNet::BufferPool<256, 1> p;
    ASSERT_FALSE(p.empty());
}

TEST(BufferPool, acquire)
{
    uNet::BufferPool<256, 1> p;
    uNet::Buffer* b = p.acquire();
    ASSERT_TRUE(b != 0);
    ASSERT_TRUE(p.empty());
}
