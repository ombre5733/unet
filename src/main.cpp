#include "kernel.hpp"
#include "networkinterface.hpp"

#include <atomic>
#include <thread>
#include <vector>

class MemoryBus;

typedef uNet::Kernel<> Kernel;

using uNet::Buffer;
using uNet::BufferPool;
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

    virtual void broadcast(Buffer &data) override;
    virtual void send(const LinkLayerAddress &address, Buffer &data) override;

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
    : NetworkInterface(0),
      m_bus(bus),
      m_name(name),
      m_hasReceivedData(false),
      m_stop(false)
{
    bus->connect(this);
}

void MemoryBusInterface::broadcast(Buffer &data)
{
    std::vector<uint8_t> rawData(data.begin(), data.end());
    m_bus->send(this, rawData);
}

void MemoryBusInterface::send(const LinkLayerAddress &address, Buffer& data)
{
    std::vector<uint8_t> rawData(data.begin(), data.end());
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

uNet::BufferPool<256, 10> pool;

void MemoryBusInterface::receive(const std::vector<uint8_t> &data)
{
    Buffer* b = pool.allocate();
    for (int i = 0; i < data.size(); ++i)
    {
        uint8_t x = data[i];
        b->push_back(x);
    }
    std::cout << m_name << " received " << b->size() << " bytes" << std::endl;
    //b->setInterface(this);
    //kernel()->receive(*b);
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

void app1(MemoryBus* bus)
{
    std::cout << "app1" << std::endl;

    Kernel k;
    MemoryBusInterface ifc(&k, "IF1", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0101, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    // Neighbor Advertisment
    Buffer* b = pool.allocate();
    {
        //NeighborSolicitation sol;
        //b.push_front((uint8_t*)&sol, sizeof(sol));
        // b.setDestinationAddress(0x

        /*
        UnetHeader h;
        h.sourceAddress = 0x0101;
        h.destinationAddress = 0x0102;
        h.nextHeader = 1;
        b->push_front((uint8_t*)&h, sizeof(h));
        */

        uint16_t datum = 0x1234;
        b->push_back(datum);
    }
    k.send(HostAddress(0x0102), b);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

void app2(MemoryBus* bus)
{
    std::cout << "app2" << std::endl;

    Kernel k;
    MemoryBusInterface ifc(&k, "IF2", bus);
    ifc.setNetworkAddress(NetworkAddress(0x0102, 0xFF00));
    k.addInterface(&ifc);
    ifc.start();

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ifc.stop();
}

#if 0
template <typename ResultT, typename ClassT>
struct bind_result
{
    typedef ResultT (ClassT::* member_function_t)();

    bind_result(member_function_t fn, ClassT* self)
        : m_memberFunction(fn),
          m_self(self)
    {
    }

    void operator() ()
    {
        (m_self->*m_memberFunction)();
    }

    member_function_t m_memberFunction;
    ClassT* m_self;
};

template <typename ResultT, typename ClassT>
bind_result<ResultT, ClassT> bind(ResultT (ClassT::* memberFunction)(), ClassT* self)
{
    return bind_result<ResultT, ClassT>(memberFunction, self);
}

struct Xyz
{
    Xyz(int i)
        : m_value(i)
    {
    }

    void print()
    {
        std::cout << "Xyz = " << m_value << std::endl;
    }

    int m_value;
};
#endif

int main()
{
    MemoryBus bus;

    std::thread t1(app1, &bus);
    std::thread t2(app2, &bus);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    t1.join();
    t2.join();
}
