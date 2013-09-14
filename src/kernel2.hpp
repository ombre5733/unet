#ifndef UNET_KERNEL2_HPP
#define UNET_KERNEL2_HPP

#include "config.hpp"

namespace uNet
{

class Cernel
{
public:

private:
    void eventLoop();

    BufferQueue m_sendQueue;
};

void Cernel::eventLoop()
{
    while (1)
    {

    }
}

} // namespace uNet

#endif // UNET_KERNEL2_HPP
