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
    MemoryBusInterface(Kernel *kernel, const std::string& name, MemoryBus* bus);

    const std::string& name() const
    {
        return m_name;
    }

    virtual void broadcast(BufferBase& data) override;
    virtual bool linkHasAddresses() const override { return false; }
    virtual void send(const LinkLayerAddress& address, BufferBase& data) override;

    void receive(const std::vector<uint8_t>& data);

    void start();
    void stop();

private:
    mutex m_receiverMutex;
    std::thread m_receiverThread;
    std::vector<uint8_t> m_receiverData;
    std::atomic<bool> m_hasReceivedData;
    std::atomic<bool> m_stop;

    void run();

    MemoryBus* m_bus;
    std::string m_name;

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
            receiver->m_receiverData = data;
            receiver->m_hasReceivedData = true;
        }
    }

private:
    weos::mutex m_mutex;
    std::vector<MemoryBusInterface*> m_interfaces;
};

MemoryBusInterface::MemoryBusInterface(Kernel* kernel, const std::string& name, MemoryBus *bus)
    : NetworkInterface(kernel),
      m_bus(bus),
      m_name(name),
      m_hasReceivedData(false),
      m_stop(false)
{
    bus->connect(this);
}

void MemoryBusInterface::broadcast(BufferBase& data)
{
    std::cout << "[" << m_name << "] broadcast - " << data.size() << std::endl;
    std::vector<uint8_t> rawData(data.begin(), data.end());
    data.dispose();
    m_bus->send(this, rawData);
}

void MemoryBusInterface::send(const LinkLayerAddress& address, BufferBase& data)
{
    std::cout << "[" << m_name << "] send - " << data.size() << std::endl;
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
            receive(m_receiverData);
            m_hasReceivedData = false;
        }
    }
}

void MemoryBusInterface::receive(const std::vector<uint8_t> &data)
{
    BufferBase* b = listener()->allocateBuffer();
    for (int i = 0; i < data.size(); ++i)
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

    // Neighbor Advertisment
    BufferBase* b = k.allocateBuffer();
    {
        uint16_t datum = 0x1234;
        b->push_back(datum);
    }
    k.send(HostAddress(0x0102), 0xFF, b);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

} // namespace app1

namespace app2
{

class PacketHandler : public uNet::CustomProtocolHandlerBase
{
public:
    virtual bool accepts(std::uint8_t headerType) const
    {
        return true;
    }

    virtual void receive(std::uint8_t headerType, BufferBase& packet)
    {
        std::cout << "[app2] received <";
        for (int i = 0; i < packet.size(); ++i)
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
    virtual bool accepts(std::uint8_t headerType) const
    {
        return true;
    }

    virtual void receive(std::uint8_t headerType, uNet::BufferBase &message)
    {
        std::cout << ">>Got a buffer of size " << message.size()
                  << " with unknown header type " << int(headerType)
                  << "<<" << std::endl;
    }
};

void test_server(uNet::SimpleMessageProtocol* smp)
{
    using namespace uNet;

    Socket skt(*smp);

    skt.listen();

    while (1)
    {
        skt.accept();

        BufferBase* b = skt.receive();

        std::cout << "<<Server>> received: ";
        for (int i = 0; i < b->size(); ++i)
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
    b->push_back(std::uint16_t(0xABCD));
    pc.dispatch(10, *b);
    b->push_back(std::uint16_t(0xEF01));
    pc.dispatch(11, *b);
    b->push_back(std::uint16_t(0x2106));
    pc.dispatch(12, *b);
#if 0
    {
        SimplePortProtocolHeader hdr;
        hdr.sourcePort = 99;
        hdr.destinationPort = 100;
        b->push_front(hdr);
        pc.dispatch(2, *b);
    }

    {
        SimpleMessageProtocolHeader hdr;
        hdr.sourcePort = 99;
        hdr.destinationPort = 100;
        b->push_front(hdr);
        pc.dispatch(254, *b);
    }
#endif
    std::thread server(test_server, pc.get<SimpleMessageProtocol>());
    std::this_thread::sleep_for(std::chrono::seconds(1));

    {
        SimpleMessageProtocolHeader hdr;
        hdr.sourcePort = 21;
        hdr.destinationPort = 42;
        b->push_front(hdr);
        pc.dispatch(254, *b);
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
