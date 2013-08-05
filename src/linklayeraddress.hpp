#ifndef LINKLAYERADDRESS_HPP
#define LINKLAYERADDRESS_HPP

#include <cstdint>

struct LinkLayerAddress
{
    LinkLayerAddress()
        : address(0)
    {
    }

    uint32_t address;
};

#endif // LINKLAYERADDRESS_HPP
