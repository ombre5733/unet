#ifndef UNET_SIMPLEPORTPROTOCOL_HPP
#define UNET_SIMPLEPORTPROTOCOL_HPP

#include "../config.hpp"

#include "../buffer.hpp"

#include <cstdint>

namespace uNet
{

template <std::uint8_t TPort, typename TService>
struct spp_port_service_map
{
    static const std::uint8_t port = TPort;
    typedef TService type;
};

namespace detail
{

template <typename TServicePortMap, typename TBaseChain>
class SppServiceChain : public TServicePortMap::type, public TBaseChain
{
public:
    void dispatch(std::uint8_t port, BufferBase& message)
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

template <typename THandlerTypes>
struct make_spp_service_chain
{
    typedef typename boost::mpl::fold<
                         THandlerTypes,
                         SppServiceChain<void, void>,
                         make_spp_service_chain_helper>::type type;

};

} // namespace detail

typedef detail::SppServiceChain<void, void> SppDefaultService;

struct SimplePortProtocolHeader
{
    std::uint8_t sourcePort;
    std::uint8_t destinationPort;
    std::uint8_t reserved[6];
};

//! A simple port-based protocol.
//!
//! SimplePortProtocol implements the handling of SPP messages.
//!
//! SimplePortProtocol implements the \p ProtocolHandler concept.
template <typename TSppServices>
class SimplePortProtocol
        : protected detail::make_spp_service_chain<TSppServices>::type
{
    typedef typename detail::make_spp_service_chain<TSppServices>::type
        service_list_t;

protected:
    //! The value of the "Next header" field in the network protocol.
    static const int headerType = 2;

    //! Returns \p true, if \p headerType equals the SPP header type.
    bool accepts(int headerType) const
    {
        return headerType == SimplePortProtocol::headerType;
    }

    //! Handles an SPP message.
    //! Handles the SPP message \p message.
    void handle(int /*headerType*/, BufferBase& message)
    {
        if (message.size() < sizeof(SimplePortProtocolHeader))
        {
            message.dispose();
            return;
        }
        std::cout << "This is a SPP message" << std::endl;

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
