#ifndef NETWORKINTERFACE_HPP
#define NETWORKINTERFACE_HPP

#include "buffer.hpp"
#include "networkaddress.hpp"

class Kernel;

class NetworkInterface
{
public:
    enum Domain
    {
        Private,
        Public
    };

    explicit NetworkInterface(Kernel* kernel);

    //! Returns the domain to which the interface is connected.
    Domain domain() const;

    Kernel* kernel() const
    {
        return m_kernel;
    }

    //! Returns the network address which has been set for this interface.
    NetworkAddress networkAddress() const
    {
        return m_networkAddress;
    }

    void setNetworkAddress(const NetworkAddress& addr);


    virtual void send(/*const PhysicalAddress& address, */Buffer& data) {}

private:
    NetworkAddress m_networkAddress;
    Kernel* m_kernel;

    typedef boost::intrusive::slist_member_hook<
        boost::intrusive::link_mode<boost::intrusive::safe_link> >
        poll_list_hook_t;
    poll_list_hook_t m_pollListHook;

    friend class Kernel;
};

#endif // NETWORKINTERFACE_HPP
