#ifndef UNET_PROTOCOL_HPP
#define UNET_PROTOCOL_HPP

#include "../buffer.hpp"

namespace uNet
{

class ProtocolSenderBase
{
public:

};

//! The base class for all protocols.
class CustomProtocolBase
{
public:
    virtual bool accepts(int headerType) const = 0;

    //! \note If the \p message is not passed on to another handler, it has
    //! to be disposed.
    virtual void receive(int headerType, BufferBase& message) = 0;

    virtual void send(ServiceBase* service, CrossLayerSendData& metaData,
                      BufferBase& message) = 0;
};

} // namespace uNet

#endif // UNET_PROTOCOL_HPP
