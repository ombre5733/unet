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

    ASSERT_TRUE(addr.isInSubnet(uNet::NetworkAddress(0x1200, 0xFF00)));
    ASSERT_TRUE(addr.isInSubnet(uNet::NetworkAddress(0x12FF, 0xFF00)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1300, 0xFF00)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x13FF, 0xFF00)));

    // If the netmask is zero, we are not in a sub-net.
    ASSERT_FALSE(addr.isInSubnet(0x1200, 0));
    ASSERT_FALSE(addr.isInSubnet(0x12FF, 0));
    ASSERT_FALSE(addr.isInSubnet(0x1300, 0));
    ASSERT_FALSE(addr.isInSubnet(0x13FF, 0));

    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1200, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x12FF, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1300, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x13FF, 0)));

    // The unspecified address is in no sub-net.
    addr = uNet::HostAddress();
    ASSERT_TRUE(addr.unspecified());
    ASSERT_FALSE(addr.isInSubnet(0x1200, 0xFF00));
    ASSERT_FALSE(addr.isInSubnet(0x12FF, 0xFF00));
    ASSERT_FALSE(addr.isInSubnet(0x1300, 0xFF00));
    ASSERT_FALSE(addr.isInSubnet(0x13FF, 0xFF00));

    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1200, 0xFF00)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x12FF, 0xFF00)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1300, 0xFF00)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x13FF, 0xFF00)));

    ASSERT_FALSE(addr.isInSubnet(0x1200, 0));
    ASSERT_FALSE(addr.isInSubnet(0x12FF, 0));
    ASSERT_FALSE(addr.isInSubnet(0x1300, 0));
    ASSERT_FALSE(addr.isInSubnet(0x13FF, 0));

    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1200, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x12FF, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x1300, 0)));
    ASSERT_FALSE(addr.isInSubnet(uNet::NetworkAddress(0x13FF, 0)));
}

TEST(HostAddress, MulticastAddresses)
{
    uNet::HostAddress addr;
    addr = uNet::HostAddress::multicastAddress();
    ASSERT_EQ(0x8000, addr);
    ASSERT_TRUE(addr.multicast());

    addr = uNet::HostAddress::multicastAddress(uNet::all_device_multicast);
    ASSERT_EQ(0x8000, addr);
    ASSERT_TRUE(addr.multicast());

    addr = uNet::HostAddress::multicastAddress(uNet::link_local_all_device_multicast);
    ASSERT_EQ(0x8001, addr);
    ASSERT_TRUE(addr.multicast());
}

// ----=====================================================================----
//     NetworkAddress
// ----=====================================================================----

TEST(NetworkAddress, Initialization)
{
    uNet::NetworkAddress addr;
    ASSERT_EQ(0, addr.hostAddress().address());
    ASSERT_EQ(0, addr.netmask());
}

