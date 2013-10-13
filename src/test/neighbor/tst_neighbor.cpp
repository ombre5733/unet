#include "../../neighbor.hpp"

#include "gtest/gtest.h"

TEST(Neighbor, Constructor)
{
    uNet::Neighbor n;
    ASSERT_EQ(uNet::Neighbor::Incomplete, n.state());
    ASSERT_TRUE(n.networkInterface() == 0);
}
