#include "kernel.hpp"
#include "networkinterface.hpp"

#include <atomic>
#include <iomanip>
#include <thread>
#include <vector>

class MemoryBus;

typedef uNet::Kernel<> Kernel;

using uNet::BufferBase;
using uNet::HostAddress;
using uNet::LinkLayerAddress;
using uNet::NetworkAddress;
using uNet::NetworkInterface;

using namespace weos;

class MemoryBusInterface : public NetworkInterface
{
public:
    MemoryBusInterface(Kernel *kernel, const char *name, MemoryBus* bus);

    virtual void broadcast(BufferBase& data) override;
    virtual bool linkHasAddresses() const override { return false; }
    virtual void send(const LinkLayerAddress& address, BufferBase& data) override;

    void receive(const std::vector<uint8_t>& data);

    void start();
    void stop();

private:
    mutex m_receiverMutex;
    std::thread m_receiverThread;
    std::vector<std::vector<uint8_t> > m_receiverData;
    std::atomic<bool> m_hasReceivedData;
    std::atomic<bool> m_stop;

    MemoryBus* m_bus;

    void run();

    friend class MemoryBus;
};

class MemoryBus
{
public:
    void connect(MemoryBusInterface* interface)
    {
        lock_guard<mutex> locker(m_mutex);
        m_interfaces.push_back(interface);
        std::cout << interface->name() << " is connected" << std::endl;
    }

    void send(MemoryBusInterface* sender, const std::vector<uint8_t>& data)
    {
        lock_guard<mutex> locker(m_mutex);

        for (std::vector<MemoryBusInterface*>::const_iterator
               iter = m_interfaces.begin(),
               end_iter = m_interfaces.end();
             iter != end_iter; ++iter)
        {
            MemoryBusInterface* receiver = *iter;
            if (receiver == sender)
                continue;

            lock_guard<mutex> locker(receiver->m_receiverMutex);
            receiver->m_receiverData.push_back(data);
            receiver->m_hasReceivedData = true;
        }
    }

private:
    weos::mutex m_mutex;
    std::vector<MemoryBusInterface*> m_interfaces;
};

MemoryBusInterface::MemoryBusInterface(Kernel* kernel, const char* name, MemoryBus *bus)
    : NetworkInterface(kernel),
      m_hasReceivedData(false),
      m_stop(false),
      m_bus(bus)
{
    setName(name);
    bus->connect(this);
}

void MemoryBusInterface::broadcast(BufferBase& data)
{
    std::cout << "[" << name() << "] broadcast - " << data.size() << std::endl;
    std::vector<uint8_t> rawData(data.begin(), data.end());
    data.dispose();
    m_bus->send(this, rawData);
}

void MemoryBusInterface::send(const LinkLayerAddress& address, BufferBase& data)
{
    std::cout << "[" << name() << "] send - " << data.size() << std::endl;
    std::vector<uint8_t> rawData(data.begin(), data.end());
    data.dispose();
    m_bus->send(this, rawData);
}

void MemoryBusInterface::run()
{
    while (!m_stop)
    {
        if (m_hasReceivedData)
        {
            lock_guard<mutex> locker(m_receiverMutex);
            for (std::size_t i = 0; i < m_receiverData.size(); ++i)
                receive(m_receiverData[i]);
            m_receiverData.clear();
            m_hasReceivedData = false;
        }
    }
}

void MemoryBusInterface::receive(const std::vector<uint8_t> &data)
{
    BufferBase* b = listener()->allocateBuffer();
    for (std::size_t i = 0; i < data.size(); ++i)
    {
        uint8_t x = data[i];
        b->push_back(x);
    }
    listener()->notify(uNet::Event::createMessageReceiveEvent(this, b));
}

void MemoryBusInterface::start()
{
    m_receiverThread = std::thread(&MemoryBusInterface::run, this);
}

void MemoryBusInterface::stop()
{
    m_stop = true;
    m_receiverThread.join();
}

namespace app1
{

void main(MemoryBus* bus)
{
    std::cout << "app1 started" << std::endl;

    Kernel k;
    MemoryBusInterface ifc(&k, "IF1", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0101, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    BufferBase* b = k.allocateBuffer();
    {
        uint16_t datum = 0x1234;
        b->push_back(datum);
    }
    k.send(HostAddress(0x0102), 0xFF, b);

#if 1
    b = k.allocateBuffer();
    {
        uint16_t datum = 0x4567;
        b->push_back(datum);
    }
    k.send(HostAddress(0x0102), 0xFF, b);
#endif

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

} // namespace app1

namespace app2
{

class PacketHandler : public uNet::CustomProtocolHandlerBase
{
public:
    virtual bool filter(const uNet::ProtocolMetaData& /*metaData*/) const
    {
        return true;
    }

    virtual void receive(const uNet::ProtocolMetaData& metaData,
                         uNet::BufferBase& packet)
    {
        std::cout << "[app2] received <";
        for (std::size_t i = 0; i < packet.size(); ++i)
        {
            if (i)
                std::cout << ' ';
            std::cout << std::hex << std::setfill('0') << std::setw(2) << int(*(packet.begin() + i)) << std::dec;
        }
        std::cout << '>' << std::endl;
    }
};

void main(MemoryBus* bus)
{
    std::cout << "app2 started" << std::endl;

    Kernel k;
    PacketHandler ph;
    k.protocolHandler<uNet::DefaultProtocolHandler>()->setCustomHandler(&ph);

    MemoryBusInterface ifc(&k, "IF2", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0102, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

} // namespace app2

#include "protocol/protocolhandlerchain.hpp"
#include "protocol/simpleportprotocol.hpp"
#include "protocol/simplemessageprotocol.hpp"

class MyBufferHandler : public uNet::CustomProtocolHandlerBase
{
public:
    virtual bool filter(const uNet::ProtocolMetaData& /*metaData*/) const
    {
        return true;
    }

    virtual void receive(const uNet::ProtocolMetaData& metaData,
                         uNet::BufferBase &message)
    {
        std::cout << ">>Got a buffer of size " << message.size()
                  << " with unknown header type " << int(metaData.npHeader.nextHeader)
                  << "<<" << std::endl;
    }
};

void test_server(uNet::SimpleMessageProtocol* smp)
{
    using namespace uNet;

    Socket skt(*smp);
    skt.bind(23);
    skt.listen();

    while (1)
    {
        skt.accept();

        BufferBase* b = skt.receive();

        std::cout << "<<Server>> received: ";
        for (std::size_t i = 0; i < b->size(); ++i)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(2) << int(*(b->begin() + i)) << std::dec << ' ';
        }
        std::cout << std::endl;

        break;
    }
}

void test_bufferhandlerchain()
{
    using namespace uNet;

    typedef boost::mpl::vector<SimpleMessageProtocol> protocol_list_t;
    //typedef boost::mpl::vector<> protocol_list_t;
    typedef make_protocol_handler_chain<protocol_list_t>::type protocols;


    protocols pc;
    MyBufferHandler h;
    pc.get<DefaultProtocolHandler>()->setCustomHandler(&h);
    //pc.get<uNet::TcpHandlerStub>()->setOption(2);

    Buffer<256, 4>* b = new Buffer<256, 4>();
    ProtocolMetaData metaData;
    b->push_back(std::uint16_t(0xABCD));
    metaData.npHeader.nextHeader = 10;
    pc.dispatch(metaData, *b);
    b->push_back(std::uint16_t(0xEF01));
    metaData.npHeader.nextHeader = 11;
    pc.dispatch(metaData, *b);
    b->push_back(std::uint16_t(0x2106));
    metaData.npHeader.nextHeader = 12;
    pc.dispatch(metaData, *b);

    std::thread server(test_server, pc.get<SimpleMessageProtocol>());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    {
        SimpleMessageProtocolHeader hdr;
        hdr.sourcePort = 21;
        hdr.destinationPort = 23;
        b->push_front(hdr);

        metaData.npHeader.nextHeader = 2;
        pc.dispatch(metaData, *b);
    }

    server.join();
    std::cout << std::endl << std::endl;
}

int main()
{
    test_bufferhandlerchain();
    std::this_thread::sleep_for(std::chrono::seconds(1));

    MemoryBus bus;

    std::thread t1(app1::main, &bus);
    std::thread t2(app2::main, &bus);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    t1.join();
    t2.join();
}
