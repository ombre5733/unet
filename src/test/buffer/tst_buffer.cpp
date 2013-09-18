#include "../../buffer.hpp"

#include "gtest/gtest.h"

#include <cstring>

class TestBufferDisposer : public uNet::BufferDisposer
{
public:
    TestBufferDisposer()
        : lastDisposedBuffer(0),
          numDisposedBuffers(0)
    {
    }

    virtual void dispose(uNet::Buffer* buffer)
    {
        lastDisposedBuffer = buffer;
        ++numDisposedBuffers;
    }

    uNet::Buffer* lastDisposedBuffer;
    int numDisposedBuffers;
};

TEST(Buffer, Initialization)
{
    {
        uNet::Buffer b;
        ASSERT_EQ(b.begin(), b.end());
        ASSERT_TRUE(b.disposer() == 0);
        ASSERT_EQ(0, b.size());
    }

    {
        TestBufferDisposer disposer;
        uNet::Buffer b(&disposer);
        ASSERT_EQ(b.begin(), b.end());
        ASSERT_TRUE(b.disposer() == &disposer);
        ASSERT_EQ(0, b.size());
    }

    //ASSERT_EQ(0, b.interface());
    //ASSERT_EQ(false, b.m_slistHook.is_linked());
}

TEST(Buffer, dispose)
{
    TestBufferDisposer disposer;
    uNet::Buffer b1(&disposer);
    uNet::Buffer b2(&disposer);
    uNet::Buffer b3(&disposer);

    b3.dispose();
    ASSERT_EQ(disposer.lastDisposedBuffer, &b3);
    ASSERT_EQ(1, disposer.numDisposedBuffers);

    b1.dispose();
    ASSERT_EQ(disposer.lastDisposedBuffer, &b1);
    ASSERT_EQ(2, disposer.numDisposedBuffers);

    b2.dispose();
    ASSERT_EQ(disposer.lastDisposedBuffer, &b2);
    ASSERT_EQ(3, disposer.numDisposedBuffers);
}

TEST(Buffer, push_back)
{
    uNet::Buffer b;
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.size());

    {
        std::uint32_t v = 0x12348765;
        b.push_back(v);
        ASSERT_FALSE(b.begin() == b.end());
        ASSERT_EQ(b.end(), b.begin() + 4);
        ASSERT_EQ(4, b.size());
        std::uint32_t w = 0;
        std::memcpy(&w, b.begin(), sizeof(w));
        ASSERT_EQ(v, w);
    }

    {
        char v = 0xAB;
        b.push_back(v);
        ASSERT_EQ(b.end(), b.begin() + 5);
        ASSERT_EQ(5, b.size());
        char w = 0;
        std::memcpy(&w, b.begin() + 4, sizeof(w));
        ASSERT_EQ(v, w);
    }

    {
        double v = 3.141592;
        b.push_back(v);
        ASSERT_EQ(b.end(), b.begin() + 13);
        ASSERT_EQ(13, b.size());
        double w = 0;
        std::memcpy(&w, b.begin() + 5, sizeof(w));
        ASSERT_EQ(v, w);
    }
}

TEST(Buffer, push_and_pop_from_front)
{
    uNet::Buffer b;
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.size());

    {
        std::uint32_t v = 0x12348765;
        b.push_front(v);
        ASSERT_FALSE(b.begin() == b.end());
        ASSERT_EQ(b.end(), b.begin() + 4);
        ASSERT_EQ(4, b.size());
        std::uint32_t w = 0;
        std::memcpy(&w, b.begin(), sizeof(w));
        ASSERT_EQ(v, w);
    }

    {
        char v = 0xAB;
        b.push_front(v);
        ASSERT_EQ(b.end(), b.begin() + 5);
        ASSERT_EQ(5, b.size());
        char w = 0;
        std::memcpy(&w, b.begin(), sizeof(w));
        ASSERT_EQ(v, w);
    }

    {
        double v = 3.141592;
        b.push_front(v);
        ASSERT_EQ(b.end(), b.begin() + 13);
        ASSERT_EQ(13, b.size());
        double w = 0;
        std::memcpy(&w, b.begin(), sizeof(w));
        ASSERT_EQ(v, w);
    }

    {
        double v = b.pop_front<double>();
        ASSERT_EQ(3.141592, v);
    }

    {
        char v = b.pop_front<char>();
        ASSERT_EQ(0xAB, v);
    }

    {
        std::uint32_t v = b.pop_front<std::uint32_t>();
        ASSERT_EQ(0x12348765, v);
    }
}
