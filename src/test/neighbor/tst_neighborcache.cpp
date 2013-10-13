#include "../../neighborcache.hpp"
#include "../../networkinterface.hpp"

#include "gtest/gtest.h"

class TestInterface : public uNet::NetworkInterface
{
public:
    TestInterface()
        : uNet::NetworkInterface(0)
    {
    }

    virtual void broadcast(uNet::BufferBase& data)
    {
    }

    virtual bool linkHasAddresses() const
    {
        return false;
    }

    virtual void send(const uNet::LinkLayerAddress& address,
                      uNet::BufferBase& data)
    {
    }
};

TEST(NeighborCache, Constructor)
{
    uNet::NeighborCache<3> nc;
    ASSERT_TRUE(nc.find(0x0101) == 0);
}

TEST(NeighborCache, createEntry)
{
    TestInterface ifc1;
    TestInterface ifc2;
    TestInterface ifc3;

    uNet::NeighborCache<3> nc;
    uNet::Neighbor* n1 = nc.createEntry(0x0101, &ifc1);
    ASSERT_TRUE(n1 != 0);
    EXPECT_TRUE(n1->networkInterface() == &ifc1);
    EXPECT_TRUE(n1->address() == 0x0101);

    // Calling find again returns the same neighbor.
    ASSERT_TRUE(nc.find(0x0101) == n1);
    ASSERT_TRUE(nc.find(0x0202) == 0);
    ASSERT_TRUE(nc.find(0x0303) == 0);

    uNet::Neighbor* n2 = nc.createEntry(0x0202, &ifc2);
    ASSERT_TRUE(n2 != 0);
    EXPECT_TRUE(n2->networkInterface() == &ifc2);
    EXPECT_TRUE(n2->address() == 0x0202);

    ASSERT_TRUE(nc.find(0x0101) == n1);
    ASSERT_TRUE(nc.find(0x0202) == n2);
    ASSERT_TRUE(nc.find(0x0303) == 0);

    uNet::Neighbor* n3 = nc.createEntry(0x0303, &ifc3);
    ASSERT_TRUE(n3 != 0);
    EXPECT_TRUE(n3->networkInterface() == &ifc3);
    EXPECT_TRUE(n3->address() == 0x0303);

    ASSERT_TRUE(nc.find(0x0101) == n1);
    ASSERT_TRUE(nc.find(0x0202) == n2);
    ASSERT_TRUE(nc.find(0x0303) == n3);
}
