#include "../../networkaddress.hpp"

#include <boost/type_traits/alignment_of.hpp>

#include "gtest/gtest.h"

TEST(HostAddress, MemoryLayout)
{
    // We require the host address to be a standard-layout class such that
    // we can copy it.
    //! \todo clarify this with is_trivial
    ASSERT_TRUE(std::is_standard_layout<uNet::HostAddress>::value);

    ASSERT_EQ(2, sizeof(uNet::HostAddress));
    ASSERT_TRUE(boost::alignment_of<uNet::HostAddress>::value <= 2);
}

TEST(HostAddress, Initialization)
{
    uNet::HostAddress addr1;
    ASSERT_EQ(0, addr1.address());

    uNet::HostAddress addr2(0x0123);
    ASSERT_EQ(0x0123, addr2.address());
}

TEST(HostAddress, ConversionOperator)
{
    uNet::HostAddress addr(0x1234);
    uint16_t u = addr;
    ASSERT_EQ(0x1234, u);
}

TEST(HostAddress, SubnetCheck)
{
    uNet::HostAddress addr(0x1234);
    ASSERT_TRUE(addr.isInSubnet(0x1200, 0xFF00));
    ASSERT_TRUE(addr.isInSubnet(0x12FF, 0xFF00));
    ASSERT_FALSE(addr.isInSubnet(0x1300, 0xFF00));
    ASSERT_FALSE(addr.isInSubnet(0x13FF, 0xFF00));
}

TEST(NetworkAddress, Initialization)
{
    uNet::NetworkAddress addr;
    ASSERT_EQ(0, addr.hostAddress().address());
    ASSERT_EQ(0, addr.netmask());
}

