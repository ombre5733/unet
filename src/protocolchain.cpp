
// A small program to test chaining of protocols.

#include <boost/mpl/fold.hpp>
#include <boost/mpl/vector.hpp>



#include <iostream>



class Ncp
{
public:
    bool accepts(int type)
    {
        return type == 0;
    }

    void handle(const char* data)
    {
        std::cout << "Ncp protocol - " << data << std::endl;
    }
};

class Tcp
{
public:
    bool accepts(int type)
    {
        return type == 1;
    }

    void handle(const char* data)
    {
        std::cout << "Tcp protocol - " << data << std::endl;
    }
};

class Udp
{
public:
    bool accepts(int type)
    {
        return type == 2;
    }

    void handle(const char* data)
    {
        std::cout << "Udp protocol - " << data << std::endl;
    }
};

template <typename TProtocol, typename TBaseChain>
class ProtocolChain : public TProtocol, public TBaseChain
{
public:
    void process(int type, const char* data)
    {
        if (TProtocol::accepts(type))
            TProtocol::handle(data);
        else
            TBaseChain::process(type, data);
    }
};

template <>
class ProtocolChain<void, void>
{
public:
    void process(int type, const char* data)
    {
    }
};

struct make_chain
{
    template <typename T1, typename T2>
    struct apply
    {
        // T1 is the result of the previous function application (or the
        // initial state in the first step) and T2 is the new type from the
        // sequence.
        typedef ProtocolChain<T2, T1> type;
    };
};

typedef boost::mpl::vector<Tcp, Udp, Ncp> ProtocolTypes;


typedef boost::mpl::fold<ProtocolTypes,
                         ProtocolChain<void, void>,
                         make_chain>::type protocol_chain_type;


int main()
{
    protocol_chain_type pc;
    pc.process(0, "NCP message");
    pc.process(1, "TCP message");
    pc.process(2, "UDP message");
}
