#include "../../networkprotocol.hpp"

#include "gtest/gtest.h"

TEST(NetworkProtocolHeader, MemoryLayout)
{
    // We require the host address to be a standard-layout class such that
    // we can copy it.
    //! \todo clarify this with is_trivial
    ASSERT_TRUE(std::is_standard_layout<uNet::NetworkProtocolHeader>::value);

    ASSERT_EQ(8, sizeof(uNet::NetworkProtocolHeader));
}

TEST(NetworkProtocolHeader, Initialization)
{
    uNet::NetworkProtocolHeader hdr;

    EXPECT_EQ(1, hdr.version);
    EXPECT_EQ(15, hdr.hopCount);

    // Copy the const static member because the assert would take its address.
    int maxHopCount = uNet::NetworkProtocolHeader::maxHopCount;
    EXPECT_EQ(15, maxHopCount);
}
