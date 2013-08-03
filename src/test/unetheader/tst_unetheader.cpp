#include "../../unetheader.hpp"

#include "gtest/gtest.h"

TEST(UnetHeader, MemoryLayout)
{
    ASSERT_EQ(true, std::is_standard_layout<UnetHeader>());

    UnetHeader hdr;
    EXPECT_EQ(8, sizeof(hdr));

    EXPECT_EQ(1, sizeof(hdr.nextHeader));
    EXPECT_EQ(2, sizeof(hdr.length));
    EXPECT_EQ(2, sizeof(hdr.sourceAddress));
    EXPECT_EQ(2, sizeof(hdr.destinationAddress));

    EXPECT_EQ(1, offsetof(UnetHeader, nextHeader));
    EXPECT_EQ(2, offsetof(UnetHeader, length));
    EXPECT_EQ(4, offsetof(UnetHeader, sourceAddress));
    EXPECT_EQ(6, offsetof(UnetHeader, destinationAddress));
}
