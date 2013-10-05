#ifndef UNET_SIMPLEPORTPROTOCOL_HPP
#define UNET_SIMPLEPORTPROTOCOL_HPP

#include "../config.hpp"

#include <cstdint>

namespace uNet
{

template <unsigned TPort, typename TService>
struct spp_port_service_map
{
    static const unsigned port = TPort;
    typedef TService type;
};

template <typename TServicePortMap, typename TBaseChain>
class SppServiceChain : public TServicePortMap::type, public TBaseChain
{
public:
    void dispatch(int port, BufferBase& message)
    {
        if (TServicePortMap::port == port)
            TServicePortMap::type::handle(message);
        else
            TBaseChain::dispatch(port, message);
    }
};

template <>
class SppServiceChain<void, void>
{
public:
    void dispatch(int port, BufferBase& message)
    {
        //  No one wants to deal with the buffer so it is discarded.
        message.dispose();
    }
};
typedef SppServiceChain<void, void> SppDefaultService;

namespace detail
{

struct make_spp_service_chain_helper
{
    template <typename T1, typename T2>
    struct apply
    {
        // T1 is the result of the previous function application (or the
        // initial state in the first step) and T2 is the new type from the
        // sequence.
        typedef SppServiceChain<T2, T1> type;
    };
};

} // namespace detail

template <typename THandlerTypes>
struct make_spp_service_chain
{
    typedef typename boost::mpl::fold<
                         THandlerTypes,
                         SppDefaultService,
                         detail::make_spp_service_chain_helper>::type type;

};

//! A simple port-based protocol.
struct SimplePortProtocolHeader
{
    std::uint8_t sourcePort;
    std::uint8_t destinationPort;
    std::uint16_t reserved;
};

template <typename TSppServices>
class SimplePortProtocolHandler : protected make_spp_service_chain<TSppServices>::type
{
    typedef typename make_spp_service_chain<TSppServices>::type service_list_t;
protected:
    static const int headerType = 20;

    bool accepts(int headerType) const
    {
        return headerType == SimplePortProtocolHandler::headerType;
    }

    void handle(int headerType, BufferBase& message)
    {
        std::cout << "This is a SPP" << std::endl;
        if (message.size() < sizeof(SimplePortProtocolHeader))
        {
            message.dispose();
            return;
        }

        const SimplePortProtocolHeader* header
                = reinterpret_cast<const SimplePortProtocolHeader*>(
                      message.begin());
        message.moveBegin(sizeof(SimplePortProtocolHeader));

        std::cout << "{SPP} src: " << int(header->sourcePort)
                  << " dest: " << int(header->destinationPort) << std::endl;

        service_list_t::dispatch(header->destinationPort, message);
    }
};

} // namespace uNet

#endif // UNET_SIMPLEPORTPROTOCOL_HPP
