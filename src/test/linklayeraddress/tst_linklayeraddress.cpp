#include "../../linklayeraddress.hpp"

#include <boost/type_traits/alignment_of.hpp>

#include "gtest/gtest.h"

TEST(LinkLayerAddress, Constructor)
{
    uNet::LinkLayerAddress lla;

    EXPECT_EQ(0, lla.address);
    EXPECT_TRUE(lla.unspecified());
}
