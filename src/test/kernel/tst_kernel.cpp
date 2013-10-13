#include "../../kernel.hpp"

#include <boost/type_traits/alignment_of.hpp>

#include "gtest/gtest.h"

class TestInterface : public uNet::NetworkInterface
{
public:
    explicit TestInterface(uNet::NetworkInterfaceListener* l)
        : uNet::NetworkInterface(l)
    {
    }

    virtual void broadcast(uNet::BufferBase& packet)
    {

    }

    virtual bool linkHasAddresses() const
    {
        return true;
    }

    virtual void send(const uNet::LinkLayerAddress& address,
                      uNet::BufferBase& packet)
    {

    }
};

class TestProtocolHandler : public uNet::CustomProtocolHandlerBase
{
public:
    virtual bool filter(const uNet::ProtocolMetaData& /*metaData*/) const
    {
        return true;
    }

    virtual void receive(const uNet::ProtocolMetaData& metaData,
                         uNet::BufferBase& packet)
    {
        //! \todo For debugging
        std::cout << "[TestProtocolHandler] received <";
        for (std::size_t i = 0; i < packet.size(); ++i)
        {
            if (i)
                std::cout << ' ';
            std::cout << std::hex << std::setfill('0') << std::setw(2) << int(*(packet.begin() + i)) << std::dec;
        }
        std::cout << '>' << std::endl;
    }

};

TEST(Kernel, Constructor)
{
    uNet::Kernel<> k;

    ASSERT_TRUE(k.protocolHandler<uNet::DefaultProtocolHandler>() != 0);
}

TEST(Kernel, allocateBuffer)
{
    uNet::Kernel<> k;
    uNet::BufferBase* b = k.allocateBuffer();
    ASSERT_TRUE(b != 0);
    b->dispose();
}

TEST(Kernel, addInterface)
{
    uNet::Kernel<> k;

    TestInterface ifc(&k);
    ASSERT_TRUE(ifc.listener() == &k);

    k.addInterface(&ifc);
}

TEST(Kernel, custom_protocol_handler)
{
    uNet::Kernel<> k;
    TestInterface ifc(&k);
    k.addInterface(&ifc);

    TestProtocolHandler ph;
    k.protocolHandler<uNet::DefaultProtocolHandler>()->setCustomHandler(&ph);
    ASSERT_TRUE(k.protocolHandler<uNet::DefaultProtocolHandler>()->customHandler() == &ph);
}
