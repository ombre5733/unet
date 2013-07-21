#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "buffer.hpp"
#include "networkinterface.hpp"
//#include "physicaladdresscache.hpp"
//#include "routingtable.hpp"

class Kernel
{
public:
    Kernel();

    //void enqueuePacket(Buffer& packet);

    void addInterface(NetworkInterface* ifc);
    void send(Buffer& packet);

private:
    std::vector<NetworkInterface*> m_interfaces;
};

#endif // KERNEL_HPP
