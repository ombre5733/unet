#ifndef UNET_CROSSLAYERDATA_HPP
#define UNET_CROSSLAYERDATA_HPP

#include "config.hpp"

#include "networkaddress.hpp"

namespace uNet
{

class CrossLayerSendData
{
    HostAddress npDestinationAddress;
    std::uint8_t destinationPort;
    std::uint8_t npNextHeaderType;
};

} // namespace uNet

#endif // UNET_CROSSLAYERDATA_HPP
