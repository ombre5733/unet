#ifndef UNET_BUFFERHANDLERCHAIN_HPP
#define UNET_BUFFERHANDLERCHAIN_HPP

#include "buffer.hpp"

#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>

namespace uNet
{

//! The service base class.
class SppServiceBase
{
public:
    virtual void accepts(int port) const = 0;
    virtual void handle(BufferBase& message) = 0;
};

class HttpServer
{
public:
    void handle(BufferBase& message)
    {
        std::cout << "Message received the HTTP server" << std::endl;
        message.dispose();
    }
};



class TcpHandlerStub
{
protected:
    static const int headerType = 10;

    bool accepts(int headerType) const
    {
        return headerType == TcpHandlerStub::headerType;
    }

    void handle(int /*headerType*/, BufferBase& message)
    {
        std::cout << "TCP protocol <" << m_option << "> - msg size: "
                  << message.size() << std::endl;
        message.dispose();
    }

public:
    void setOption(int option)
    {
        m_option = option;
    }

private:
    int m_option;
};

class UdpHandlerStub
{
protected:
    static const int headerType = 11;

    UdpHandlerStub()
        : m_option(0)
    {
    }

    bool accepts(int headerType) const
    {
        return headerType == UdpHandlerStub::headerType;
    }

    void handle(int /*headerType*/, BufferBase& message)
    {
        std::cout << "UDP protocol <" << m_option << "> - msg size: "
                  << message.size() << std::endl;
        message.dispose();
    }

public:
    void setOption(int option)
    {
        m_option = option;
    }

private:
    int m_option;
};

//! The base class of all buffer handlers.
//! The BufferHandlerBase is the base class of all buffer handlers.
class BufferHandlerBase
{
public:
    virtual bool accepts(int headerType) const = 0;

    //! \note If the \p message is not passed on to another handler, it has
    //! to be disposed.
    virtual void handle(int headerType, BufferBase& message) = 0;
};

template <typename THandler, typename TBaseChain>
class BufferHandlerChain : public THandler, public TBaseChain
{
public:
    void dispatch(int headerType, BufferBase& message)
    {
        if (THandler::accepts(headerType))
            THandler::handle(headerType, message);
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
class BufferHandlerChain<void, void>
{
public:
    BufferHandlerChain()
        : m_customHandler(0)
    {
    }

    void dispatch(int headerType, BufferBase& message)
    {
        if (m_customHandler && m_customHandler->accepts(headerType))
            m_customHandler->handle(headerType, message);
        else
        {
            //  No one wants to deal with the buffer so it is discarded.
            message.dispose();
        }
    }

    void setCustomHandler(BufferHandlerBase* handler)
    {
        m_customHandler = handler;
    }

private:
    BufferHandlerBase* m_customHandler;
};

typedef BufferHandlerChain<void, void> DefaultBufferHandler;

namespace detail
{

struct make_buffer_handler_chain_helper
{
    template <typename T1, typename T2>
    struct apply
    {
        // T1 is the result of the previous function application (or the
        // initial state in the first step) and T2 is the new type from the
        // sequence.
        typedef BufferHandlerChain<T2, T1> type;
    };
};

} // namespace detail

//! \code
//! typedef boost::mpl::vector<Tcp, Udp, Ncp> ProtocolTypes;
//! make_buffer_handler_chain<ProtocolTypes>::type handlerChain;
//! \endcode
template <typename THandlerTypes>
struct make_buffer_handler_chain
{
    typedef typename boost::mpl::fold<
                         THandlerTypes,
                         DefaultBufferHandler,
                         detail::make_buffer_handler_chain_helper>::type type;

};

} // namespace uNet

#endif // UNET_BUFFERHANDLERCHAIN_HPP
