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
    typedef boost::intrusive::member_hook<
            Buffer,
            Buffer::slist_hook_t,
            &Buffer::m_slistHook> list_options;
    typedef boost::intrusive::slist<
            Buffer,
            list_options,
            boost::intrusive::cache_last<true> > BufferList;

    BufferList l;
    Buffer b;
    l.push_back(b);
}
