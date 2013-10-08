#ifndef UNET_KERNELBASE_HPP
#define UNET_KERNELBASE_HPP

#include "config.hpp"

#include "buffer.hpp"
#include "networkaddress.hpp"

#include <cstdint>

namespace uNet
{

//! The base class for all kernels.
//! KernelBase is an abstract base class for all kernels.
//! \todo Merge with NetworkInterfaceListener
class KernelBase
{
public:
    /*
    virtual void broadcast(std::uint8_t headerType, BufferBase& packet) = 0;

    virtual void broadcastOnLink(NetworkInterface* ifc, std::uint8_t headerType,
                                 BufferBase& packet);
    */

    virtual void send(HostAddress destination, std::uint8_t headerType,
                      BufferBase& packet) = 0;

protected:

};

} // namespace uNet

#endif // UNET_KERNELBASE_HPP
