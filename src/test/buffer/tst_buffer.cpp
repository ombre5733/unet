#include "../../buffer.hpp"

#include "gtest/gtest.h"

TEST(Buffer, Initialization)
{
    Buffer b;
    ASSERT_EQ(b.begin(), b.end());
    ASSERT_EQ(0, b.size());

    ASSERT_EQ(0, b.interface());
    //ASSERT_EQ(false, b.m_slistHook.is_linked());
}

TEST(BufferList, Alle)
{
    BufferList l;
    Buffer b;
    l.push_back(b);
    l.pop_front();
}
