#include "../buffer.hpp"

#include "gtest/gtest.h"

TEST(BufferQueue, Initialization)
{
    uNet::BufferQueue q;
    ASSERT_TRUE(q.empty());
}

TEST(BufferQueue, push_and_pop)
{
    uNet::BufferQueue q;
    uNet::Buffer<256, 1> b1;
    uNet::Buffer<128, 2> b2;
    uNet::Buffer< 64, 4> b3;

    q.push_front(b1);
    ASSERT_EQ(&b1, &q.front());
    ASSERT_EQ(&b1, &q.back());
    ASSERT_FALSE(q.empty());

    q.push_back(b2);
    ASSERT_EQ(&b1, &q.front());
    ASSERT_EQ(&b2, &q.back());
    ASSERT_FALSE(q.empty());

    q.push_front(b3);
    ASSERT_EQ(&b3, &q.front());
    ASSERT_EQ(&b2, &q.back());
    ASSERT_FALSE(q.empty());

    q.pop_front();
    ASSERT_EQ(&b1, &q.front());
    ASSERT_EQ(&b2, &q.back());
    ASSERT_FALSE(q.empty());

    q.push_back(b3);
    ASSERT_EQ(&b1, &q.front());
    ASSERT_EQ(&b3, &q.back());
    ASSERT_FALSE(q.empty());

    q.pop_front();
    ASSERT_EQ(&b2, &q.front());
    ASSERT_EQ(&b3, &q.back());
    ASSERT_FALSE(q.empty());

    q.pop_front();
    ASSERT_EQ(&b3, &q.front());
    ASSERT_EQ(&b3, &q.back());
    ASSERT_FALSE(q.empty());

    q.pop_front();
    ASSERT_TRUE(q.empty());
}
