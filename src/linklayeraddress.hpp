#ifndef UNET_LINKLAYERADDRESS_HPP
#define UNET_LINKLAYERADDRESS_HPP

#include <cstdint>

namespace uNet
{

//! A link-layer address.
//! A LinkLayerAddress represents the address of an interface on the physical
//! link. The size and contents of such an address is specific to the link.
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
