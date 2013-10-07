#ifndef UNET_PROTOCOL_HPP
#define UNET_PROTOCOL_HPP

#include "../config.hpp"

#include "../buffer.hpp"
#include "../networkprotocol.hpp"

#include <cstdint>

namespace uNet
{

//! The base class for all custom protocol handlers.
class CustomProtocolHandlerBase
{
public:
    //! Destroys the protocol handler.
    virtual ~CustomProtocolHandlerBase() {}

    //! Checks if an incoming packet is accepted by the protocol.
    //! Derived classes have to implement this method to filter incoming
    //! packets by their network protocol \p header. If \p true is returned,
    //! the packet is passed to receive().
    virtual bool filter(const NetworkProtocolHeader& header) const = 0;

    //! Deals with an incoming packet.
    //! Handles the incoming \p packet with the given network protocol
    //! \p header. This method is only called, if filter() returned true.
    //!
    //! \note If the \p packet is not passed on to another handler, it has
    //! to be disposed.
    virtual void receive(const NetworkProtocolHeader& header,
                         BufferBase& packet) = 0;

    /*
    virtual void send(ServiceBase* service, CrossLayerSendData& metaData,
                      BufferBase& message) = 0;
    */
};

} // namespace uNet

#endif // UNET_PROTOCOL_HPP
