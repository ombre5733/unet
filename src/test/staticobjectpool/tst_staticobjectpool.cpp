#include "../../staticobjectpool.hpp"

#include "gtest/gtest.h"

TEST(StaticObjectPool, Initialization)
{
    StaticObjectPool<int, 1> pool;
    ASSERT_EQ(false, pool.empty());
}
