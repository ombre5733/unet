#ifndef UNETHEADER_HPP
#define UNETHEADER_HPP

#include <cstdint>


// The uNet network protocol has the following header
// 0                   1                   2                   3
// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |Version|HopCnt |  Next header  |            Length             |
// +-------+-------+---------------+---------------+---------------+
// |            Source             |          Destination          |
// +---------------+---------------+---------------+---------------+

struct UnetHeader
{
    uint8_t version : 4;
    uint8_t hopCount : 4;
    uint8_t nextHeader;
    uint16_t length;
    uint16_t sourceAddress;
    uint16_t destinationAddress;

    static const uint8_t maxHopCount = 15;
};

#include <iostream>

inline
std::ostream& operator<< (std::ostream& os, const UnetHeader& hdr)
{
    os << std::hex << hdr.sourceAddress << " "
       << std::hex << hdr.destinationAddress
       << std::dec;
    return os;
}

#endif // UNETHEADER_HPP
