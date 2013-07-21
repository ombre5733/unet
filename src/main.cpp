// Address: device.net.host
// 0 .. local device/net/host
//      0.2.1 .. host 1 in net 2 in local device
//      0.0.3 .. host 3 in local net in local device
//      0.0.0 .. local host (in local net in local device)
//      0.1.0 .. invalid
//      1.0.0 .. invalid
// F .. all devices/nets/host
//      1.2.F .. everyone in net 2 in device 1
//      1.F.F .. everyone in device 1
//      0.0.F .. everyone in local net
//      0.F.F .. everyone in local device

// relay agent
//    Does not forward a packet transparently but modifies a packet before
//    forwarding. A relay agent is required, e.g. when a host does not see
//    a DHCP client in its own net. The relay may forward the requests to
//    a DHCP in another net and translate the messages back to the client.

// Link-layer
//   Error detection (checksum/CRC)

// IP-like header
//   Version               ( 4 bit)
//   Source Address        (16 bit)
//   Destination Address   (16 bit)
//   Next header type      ( 8 bit)
//   Length                (16 bit)
//                  Total:  60 bit
//   No chechsum/CRC because this is handled by the link

// TCP-like
//   Source port           (16 bit)
//   Destination port      (16 bit)
//   Sequence number         8 bit
//   Acknowledge number      8 bit
//   Flags                   8 bit
//                 Total:   56 bit

// UDP-like
//   Source port           (16 bit)
//   Destination port      (16 bit)

// SCTP-like
//   Source port
//   Destination port
//   Chunk1-Type Chunk1-Flags Chunk1-Length Chunk1-Data
//   Chunk2-Type Chunk2-Flags Chunk2-Length Chunk2-Data
//   ...
// Chunk-Flags: Start of Fragment, End of Fragment usw. Abhängig vom Chunk-Type
//
// Während TCP einen Byte-Strom an Daten rausschickt, schickt SCTP die Daten
// in Chunks, die mit einer Sequence-Number versehen sind. Die SN ist
// während der Übertragung einer fragmentierten Nachricht für alle Fragmente
// gleich. SCTP gibt an die Applikation eine vollständige Nachricht weiter,
// d.h. die Applikation muss nicht wie bei TCP die Bytes abzählen.


// Address assignment (RFC1541 - DHCP):
//
//   The DHCP server can send a FORCERENEW message to a host in order to force
//   a renewal of the IP-address.


#include <vector>

#include "kernel.hpp"

#if 0
class CommunicationBufferPool
{
public:
    NetworkBuffer& acquireBuffer();
};

//! Simply a host address.
//!
//! 16 bit
//! 0000 ... local
//! 0001 ... multicast
//! 0010 ... global
class HostAddress
{
private:
    uint16_t m_address;
};

//! A complete network address consisting of the host address and the netmask.
class NetworkAddress
{
public:
private:
    uint16_t m_address;
    uint16_t m_netmask;
};

// Disconnected: Not connected.
// Connected: Connected but not yet functional.
// Configured: Functional
class NetworkInterface
{
public:
    enum State
    {
        Disconnected,
        Connected,
        Configured
    };

    //! Returns the network address which has been set for this interface.
    NetworkAddress networkAddress() const;

    //! Returns the state of the link.
    virtual State state() const { return Configured; }
    virtual bool send(IpFragment* fragment) { return true; }

    //! Sends a message to all devices on the same link.
    virtual bool sendBroadcast(IpFragment* fragment) { return true; }
};

class DummyInterface : public NetworkInterface
{
public:
};

class NetworkManager
{
public:

private:
    std::vector<NetworkBuffer*> m_pendingBuffers;
};

void NetworkProtocolStack::receiveFragment(NetworkBuffer* buffer)
{
    if (buffer->length() < sizeof(NetworkProtocolHeader))
        return;

    const NetworkProtocolHeader* hdr
            = static_cast<const NetworkProtocolHeader*>(buffer->data());
    // If the packet does not belong to the interface which received it,
    // it has to be routed.
    if (buffer->interface()->networkAddress() != hdr->destinationAddress)
    {
        route(buffer);
        return;
    }


}


class IpFragment
{
public:
    void release();
};

class IpRouter
{
public:
    IpFragment* fragment();

    void handleReceivedFragment(IpFragment& fragment);
};

class IpRoutingTable
{
public:
};

class TransportProtocolHandler
{
};

class TcpHandler : public TransportProtocolHandler
{
};

class UdpHandler : public TransportProtocolHandler
{
};

class TcpService
{
};

class Dhcp : public TcpService
{
public:

};

class RemoteProcedureCallServer
{
};
#endif

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





// A (1)
// |
// |
// B (2)
// |
// +--------+
// |        |
// C (3)    D (4)
//          |
//          |
//          E (5)


// Normalerweise numeriert IP jedes Interface. Die Knoten B, C, D und E hätten
// demnach zwei Addressen.








