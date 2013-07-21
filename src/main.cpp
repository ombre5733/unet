#include "unetheader.hpp"
#include "networkinterface.hpp"

int main()
{

    uint16_t myHostAddr = 0x0103;

    Kernel kernel;

    NetworkInterface ifc;
    ifc.setNetworkAddress(NetworkAddress(myHostAddr, 0xFF00));
    kernel.addInterface(&ifc);

    UnetHeader h;
    h.sourceAddress = myHostAddr;
    h.destinationAddress = 0x0107;

    Buffer b;
    b.push_front((uint8_t*)&h, sizeof(h));
    kernel.send(b);

}
