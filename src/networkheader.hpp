#ifndef UNET_NETWORKHEADER_HPP
#define UNET_NETWORKHEADER_HPP

#include <cstdint>

namespace uNet
{

// The uNet network protocol has the following header
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Version|HopCnt |  Next header  |            Length             |
// +-------+-------+---------------+---------------+---------------+
// |            Source             |          Destination          |
// +---------------+---------------+---------------+---------------+

struct NetworkHeader
{
    std::uint8_t version : 4;
    std::uint8_t hopCount : 4;
    std::uint8_t nextHeader;
    std::uint16_t length;
    std::uint16_t sourceAddress;
    std::uint16_t destinationAddress;

    static const std::uint8_t maxHopCount = 15;
};

} namespace uNet

#endif // UNET_NETWORKHEADER_HPP
