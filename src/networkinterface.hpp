#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include "buffer.hpp"
#include "networkaddress.hpp"

class NetworkInterface
{
public:
    enum Domain
    {
        Private,
        Public
    };

    NetworkInterface();

    //! Returns the domain to which the interface is connected.
    Domain domain() const;

    //! Returns the network address which has been set for this interface.
    NetworkAddress networkAddress() const
    {
        return m_networkAddress;
    }

    void setNetworkAddress(const NetworkAddress& addr);


    void send(/*const PhysicalAddress& address, */const Buffer& data);

private:
    NetworkAddress m_networkAddress;
};

#endif // NETWORKINTERFACE_HPP
