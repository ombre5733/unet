#ifndef UNET_LINKLAYERADDRESS_HPP
#define UNET_LINKLAYERADDRESS_HPP

#include <cstdint>

namespace uNet
{

struct LinkLayerAddress
{
    LinkLayerAddress()
        : address(0)
    {
    }

    std::uint32_t address;
};

} // namespace uNet

#endif // UNET_LINKLAYERADDRESS_HPP
