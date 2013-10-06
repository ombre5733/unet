#ifndef UNET_PROTOCOLHANDLERCHAIN_HPP
#define UNET_PROTOCOLHANDLERCHAIN_HPP

#include "../config.hpp"

#include "protocol.hpp"

#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

#include <cstdint>

namespace uNet
{

template <typename THandler, typename TBaseChain>
class ProtocolHandlerChain : public THandler, public TBaseChain
{
public:
    void dispatch(std::uint8_t headerType, BufferBase& message)
    {
        if (THandler::accepts(headerType))
            THandler::receive(headerType, message);
        else
            TBaseChain::dispatch(headerType, message);
    }

    //! Returns a pointer to a handler in this handler chain.
    template <typename CastT>
    CastT* get()
    {
        return static_cast<CastT*>(this);
    }

    //! Returns a pointer to a handler in this handler chain.
    template <typename CastT>
    const CastT* get() const
    {
        return static_cast<const CastT*>(this);
    }
};

template <>
class ProtocolHandlerChain<void, void>
{
public:
    ProtocolHandlerChain()
        : m_customHandler(0)
    {
    }

    void dispatch(std::uint8_t headerType, BufferBase& message)
    {
        if (m_customHandler && m_customHandler->accepts(headerType))
            m_customHandler->receive(headerType, message);
        else
        {
            //  No one wants to deal with the buffer so it is discarded.
            message.dispose();
        }
    }

    void setCustomHandler(CustomProtocolHandlerBase* handler)
    {
        m_customHandler = handler;
    }

private:
    CustomProtocolHandlerBase* m_customHandler;
};

typedef ProtocolHandlerChain<void, void> DefaultProtocolHandler;

namespace detail
{

struct make_protocol_handler_chain_helper
{
    template <typename T1, typename T2>
    struct apply
    {
        // T1 is the result of the previous function application (or the
        // initial state in the first step) and T2 is the new type from the
        // sequence.
        typedef ProtocolHandlerChain<T2, T1> type;
    };
};

} // namespace detail

//! \code
//! typedef boost::mpl::vector<Tcp, Udp, Ncp> ProtocolTypes;
//! make_protocol_handler_chain<ProtocolTypes>::type handlerChain;
//! \endcode
template <typename THandlerTypes>
struct make_protocol_handler_chain
{
    typedef typename boost::mpl::fold<
                         THandlerTypes,
                         DefaultProtocolHandler,
                         detail::make_protocol_handler_chain_helper>::type type;

};

} // namespace uNet

#endif // UNET_PROTOCOLHANDLERCHAIN_HPP
