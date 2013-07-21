#include "kernel.hpp"
#include "unetheader.hpp"

#include <cassert>


Kernel::Kernel()
{
}

void Kernel::addInterface(NetworkInterface *ifc)
{
    m_interfaces.push_back(ifc);
}

void Kernel::send(Buffer &packet)
{
    if (packet.size() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << *header << std::endl;

    HostAddress destAddr(header->destinationAddress);
    // Perform a look-up in the destination cache.
    // ... todo ...

    // We have not found an entry in the destination cache.
    //determineNextHop();

    for (size_t i = 0; i < m_interfaces.size(); ++i)
        if (destAddr.isInSubnet(m_interfaces[i]->networkAddress()))
        {
            std::cout << "belongs to interface no " << i << std::endl;
        }
}

void Kernel_receive(Buffer& packet)
{
    if (packet.size() < sizeof(UnetHeader))
        return;
    const UnetHeader* header
            = reinterpret_cast<const UnetHeader*>(packet.begin());
    std::cout << *header << std::endl;

    assert(packet.interface());

    HostAddress destAddr(header->destinationAddress);
    if (destAddr == packet.interface()->networkAddress().hostAddress())
    {
        // The packet belongs to the interface and as such it needs to be
        // dispatched.
    }
    else
    {
        // The packet has to be routed.
    }
}





class DestinationCache
{
public:
    // std::map<HostAddress/*destination*/, HostAddress>
};
