#include "../../networkinterface.hpp"

#include "gtest/gtest.h"

#include <cstring>

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

    virtual void send(const uNet::LinkLayerAddress& address,
                      uNet::BufferBase& data)
    {
    }
};

TEST(NetworkInterface, Initialization)
{
    TestInterface ifc;

    ASSERT_TRUE(ifc.networkAddress().hostAddress().unspecified());
}
