#include "../../buffer.hpp"

#include "gtest/gtest.h"

TEST(Buffer, Initialization)
{
    Buffer b;
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.interface());
}
