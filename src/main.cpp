#include "kernel.hpp"
#include "networkinterface.hpp"
#include "protocol/simplemessageprotocol.hpp"

#include <atomic>
#include <iomanip>
#include <thread>
#include <vector>

class MemoryBus;

struct app_kernel_traits : public uNet::default_kernel_traits
{
    //! A list of protocols which are attached to the kernel.
    typedef boost::mpl::vector<uNet::SimpleMessageProtocol> protocol_list_t;
};

typedef uNet::Kernel<app_kernel_traits> Kernel;

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

void test_client(Kernel& k)
{
    using namespace uNet;

    SendSocket skt(*k.protocolHandler<uNet::SimpleMessageProtocol>(), 21);
    SendConnection connection = skt.connect(0x0102, 23);
    BufferBase* b = k.allocateBuffer();
    {
        uint16_t datum = 0x1234;
        b->push_back(datum);
    }
    connection.send(b);

    b = k.allocateBuffer();
    {
        uint16_t datum = 0x4567;
        b->push_back(datum);
    }
    connection.send(b);
}

void main(MemoryBus* bus)
{
    std::cout << "app1 started" << std::endl;

    Kernel k;
    MemoryBusInterface ifc(&k, "IF1", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0101, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    test_client(k);

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

void test_server(Kernel& k)
{
    using namespace uNet;

    ReceiveSocket<1> skt(*k.protocolHandler<uNet::SimpleMessageProtocol>(), 23);

    int messageCounter = 0;
    while (1)
    {
        ReceiveConnection connection = skt.accept();

        BufferBase* b = connection.receive();
        ++messageCounter;

        std::cout << "<<Server>> received: ";
        for (std::size_t i = 0; i < b->size(); ++i)
        {
            std::cout << std::hex << std::setfill('0') << std::setw(2) << int(*(b->begin() + i)) << std::dec << ' ';
        }
        std::cout << std::endl;

        b->dispose();
        break;
    }
}

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

    test_server(k);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

} // namespace app2

int main()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));

    MemoryBus bus;

    std::thread t1(app1::main, &bus);
    std::thread t2(app2::main, &bus);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    t1.join();
    t2.join();
}
