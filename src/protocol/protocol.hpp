#ifndef UNET_PROTOCOL_HPP
#define UNET_PROTOCOL_HPP

#include "../config.hpp"

#include "../buffer.hpp"

#include <cstdint>

namespace uNet
{

//! The base class for all custom protocol handlers.
class CustomProtocolHandlerBase
{
public:
    //! Destroys the protocol handler.
    virtual ~CustomProtocolHandlerBase() {}

    //! Checks if a packet type is accepted by the protocol.
    //! Derived classes have to implement this method. It is called for
    //! every incoming packet. THe "Next header" field of the network protocol
    //! is passed in \p headerType. If the return value is \p true, the
    //! packet is passed to receive().
    virtual bool accepts(std::uint8_t headerType) const = 0;

    //! Deals with a received packet.
    //! Handles the incoming \p packet whose network protocol field
    //! "Next header" is passed in \p headerType.
    //!
    //! \note If the \p packet is not passed on to another handler, it has
    //! to be disposed.
    virtual void receive(std::uint8_t headerType, BufferBase& packet) = 0;

    /*
    virtual void send(ServiceBase* service, CrossLayerSendData& metaData,
                      BufferBase& message) = 0;
    */

private:
};

} // namespace uNet

#endif // UNET_PROTOCOL_HPP
