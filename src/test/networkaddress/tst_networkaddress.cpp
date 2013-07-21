#include "../../networkaddress.hpp"

#include "gtest/gtest.h"

TEST(HostAddress, Initialization)
{
    HostAddress addr1;
    ASSERT_EQ(0, addr1.address());

    HostAddress addr2(0x0123);
    ASSERT_EQ(0x0123, addr2.address());
}

TEST(HostAddress, ConversionOperator)
{
    HostAddress addr(0x1234);
    uint16_t u = addr;
    ASSERT_EQ(0x1234, u);
}

TEST(HostAddress, SubnetCheck)
{
    HostAddress addr(0x1234);
    ASSERT_EQ(true, addr.isInSubnet(0x1200, 0xFF00));
    ASSERT_EQ(true, addr.isInSubnet(0x12FF, 0xFF00));
    ASSERT_EQ(false, addr.isInSubnet(0x1300, 0xFF00));
    ASSERT_EQ(false, addr.isInSubnet(0x13FF, 0xFF00));
}

TEST(NetworkAddress, Initialization)
{
    NetworkAddress addr;
    ASSERT_EQ(0, addr.hostAddress().address());
    ASSERT_EQ(0, addr.netmask());
}
