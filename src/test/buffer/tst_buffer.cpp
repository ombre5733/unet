#include "../../buffer2.hpp"

#include "gtest/gtest.h"

TEST(Buffer, Initialization)
{
    Buffer2 b;
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.size());

    //ASSERT_EQ(0, b.interface());
    //ASSERT_EQ(false, b.m_slistHook.is_linked());
}

TEST(BufferHandle, Initialization)
{
    Buffer2 b;
    BufferHandle h(&b);
}

/*
TEST(BufferQueue, Alle)
{
    Buffer2Queue l;
    Buffer2 b;
    l.push_back(b);
    l.pop_front();
}
*/
