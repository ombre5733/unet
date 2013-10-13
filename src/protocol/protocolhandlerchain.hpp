#ifndef UNET_PROTOCOLHANDLERCHAIN_HPP
#define UNET_PROTOCOLHANDLERCHAIN_HPP

#include "../config.hpp"

#include "protocol.hpp"

#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

#include <cstdint>

namespace uNet
{
class KernelBase;

//! A class for chaining protocol handlers.
//! The ProtocolHandlerChain inherits a protocol handler and a base chain.
template <typename THandler, typename TBaseChain>
class ProtocolHandlerChain : public THandler, public TBaseChain
{
public:
    //! Dispatches an incoming packet.
    //! Dispatches the incoming \p packet with an associated network protocol
    //! \p metaData. If the protocol handler \p THandler accepts the packet
    //! (\p filter() returns \p true), the packet is passed on to it.
    //! Otherwise, the dispatch() method inherited from the base chain is
    //! called.
    void dispatch(const ProtocolMetaData& metaData, BufferBase& packet)
    {
        if (THandler::filter(metaData))
            THandler::receive(metaData, packet);
        else
            TBaseChain::dispatch(metaData, packet);
    }

    //! Returns a pointer to a handler in this chain.
    template <typename CastT>
    CastT* cast()
    {
        return static_cast<CastT*>(this);
    }

    //! Returns a pointer to a handler in this chain.
    template <typename CastT>
    const CastT* cast() const
    {
        return static_cast<const CastT*>(this);
    }

    void setKernel(KernelBase* kernel)
    {
        THandler::setKernel(kernel);
        TBaseChain::setKernel(kernel);
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

    //! Dispatches an incoming packet.
    //! Dispatches the incoming \p packet with its associated network protocol
    //! \p metaData.
    //! If a custom protocol handler has been set and it accepts the \p header,
    //! the packet is passed on to it. Otherwise, the packet is disposed.
    void dispatch(const ProtocolMetaData& metaData, BufferBase& packet)
    {
        if (m_customHandler && m_customHandler->filter(metaData))
            m_customHandler->receive(metaData, packet);
        else
        {
            //  No one wants to deal with the packet so it is discarded.
            packet.dispose();
        }
    }

    //! Returns a pointer to a handler in this chain.
    template <typename CastT>
    CastT* cast()
    {
        return static_cast<CastT*>(this);
    }

    //! Returns a pointer to a handler in this chain.
    template <typename CastT>
    const CastT* cast() const
    {
        return static_cast<const CastT*>(this);
    }

    //! Returns the custom protocol handler.
    //! Returns the custom protocol handler which has been assigned.
    CustomProtocolHandlerBase* customHandler() const
    {
        return m_customHandler;
    }

    //! Sets a custom protocol handler.
    //! Assigns a custom protocol handler \p handler.
    void setCustomHandler(CustomProtocolHandlerBase* handler)
    {
        m_customHandler = handler;
    }

    void setKernel(KernelBase* /*kernel*/)
    {
    }

private:
    //! The custom protocol handler.
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
