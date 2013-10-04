#include "kernel.hpp"
#include "networkinterface.hpp"

#include <atomic>
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

void handleMessage(BufferBase* b)
{
    std::cout << "[app2] received " << b->size() << " bytes" << std::endl;
}

void main(MemoryBus* bus)
{
    std::cout << "app2 started" << std::endl;

    Kernel k;
    k.messageReceivedCallback = handleMessage;

    MemoryBusInterface ifc(&k, "IF2", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0102, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

} // namespace app2

#include "bufferhandlerchain.hpp"

class MyBufferHandler : public uNet::BufferHandler
{
public:
    virtual bool accepts(int headerType) const
    {
        return true;
    }

    virtual void handle(int headerType, uNet::BufferBase &message)
    {
        std::cout << ">>Got a buffer of size " << message.size()
                  << " with unknown header type " << headerType
                  << "<<" << std::endl;
    }
};

void test_bufferhandlerchain()
{
    using namespace uNet;

    typedef boost::mpl::vector<TcpHandlerStub, UdpHandlerStub> protocol_list_t;
    typedef make_buffer_handler_chain<protocol_list_t>::type chain;

    chain c;
    MyBufferHandler h;
    c.get<DefaultBufferHandler>()->setCustomHandler(&h);
    c.get<uNet::TcpHandlerStub>()->setOption(2);

    Buffer<256, 4>* b = new Buffer<256, 4>();
    b->push_back(std::uint16_t(0xABCD));
    c.dispatch(10, *b);
    b->push_back(std::uint16_t(0xEF01));
    c.dispatch(11, *b);
    b->push_back(std::uint16_t(0x2106));
    c.dispatch(12, *b);

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
