#include "../../buffer2.hpp"

#include "gtest/gtest.h"

class BufferDisposerStub : public uNet::BufferDisposer
{
public:
    virtual void dispose(uNet::Buffer2* /*buffer*/)
    {
    }
};

TEST(Buffer, Initialization)
{
    BufferDisposerStub disposer;
    uNet::Buffer2 b(&disposer);
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.size());

    //ASSERT_EQ(0, b.interface());
    //ASSERT_EQ(false, b.m_slistHook.is_linked());
}

#if 0
#include "../staticobjectpool.hpp"



template <unsigned TBufferSize>
class BufferPool : public BufferDisposer
{
public:
    typedef Buffer2 buffer_type;

    buffer_type* acquire()
    {
        std::cout << "BufferPool::acquire" << std::endl;
        return m_pool.construct(this);
    }

    void release(buffer_type* const buffer)
    {
        std::cout << "BufferPool::release" << std::endl;
        m_pool.destroy(buffer);
    }

protected:
    virtual void operator() (buffer_type* buffer)
    {
        release(buffer);
    }

private:
    StaticObjectPool<buffer_type, 10> m_pool;
};


TEST(Pooling, X)
{
    BufferPool<256> p;
    Buffer2* b = p.acquire();

    b->dispose();

}
#endif
