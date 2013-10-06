#ifndef UNET_KERNELBASE_HPP
#define UNET_KERNELBASE_HPP

#include "config.hpp"

#include "buffer.hpp"
#include "networkaddress.hpp"

#include <cstdint>

namespace uNet
{

struct KernelTransmitData
{
    HostAddress destinationAddress;
    std::uint8_t nextHeaderType;
};

struct SppTransmitData : public KernelTransmitData
{
    std::uint8_t sourcePort;
    std::uint8_t destinationPort;
};

//! The base class for all kernels.
//! KernelBase is an abstract base class for all kernels.
//! \todo Merge with NetworkInterfaceListener
class KernelBase
{
public:
    /*
    virtual void broadcast(std::uint8_t headerType, BufferBase& message) = 0;

    virtual void broadcastOnLink(NetworkInterface* ifc, std::uint8_t headerType,
                                 BufferBase& message);
    */

    virtual void send(HostAddress destination, std::uint8_t headerType,
                      BufferBase& message) = 0;
};

} // namespace uNet

#endif // UNET_KERNELBASE_HPP
