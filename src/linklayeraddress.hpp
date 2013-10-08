#ifndef UNET_LINKLAYERADDRESS_HPP
#define UNET_LINKLAYERADDRESS_HPP

#include <cstdint>

namespace uNet
{

//! A link-layer address.
//! A LinkLayerAddress represents the address of an interface on the physical
//! link. The size and contents of such an address is specific to the link's
//! interface implementation.
//! The LinkLayerAddress has enough storage to hold any link address. The
//! only requirement is that a zero value represents an unspecified address.
struct LinkLayerAddress
{
    LinkLayerAddress()
        : address(0)
    {
    }

    //! Checks if this is an unspecified address.
    //! Returns \p true, if this link-layer address is an unspecified address,
    //! i.e. if it equals 0.
    bool unspecified() const
    {
        return address == 0;
    }

    std::uint32_t address;
};

} // namespace uNet

#endif // UNET_LINKLAYERADDRESS_HPP
