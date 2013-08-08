#include "../../neighborcache.hpp"

#include "gtest/gtest.h"

TEST(TimeoutList, Initialization)
{
    TimeoutList l;
    ASSERT_EQ(true, l.empty());
}

TEST(TimeoutList, OrderedInsert)
{
    TimeoutList l;

    Neighbor n1;
    l.insert(n1, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());

    Neighbor n2;
    l.insert(n2, 12);
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());

    Neighbor n3;
    l.insert(n3, 8);
    ASSERT_EQ(l.iterator_to(n3), l.begin());
    ASSERT_EQ(l.iterator_to(n1), ++l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++ ++l.begin());

    l.clear();
}

TEST(TimeoutList, Fifo)
{
    TimeoutList l;

    Neighbor n1;
    l.insert(n1, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());

    Neighbor n2;
    l.insert(n2, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());

    Neighbor n3;
    l.insert(n3, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());
    ASSERT_EQ(l.iterator_to(n3), ++ ++l.begin());

    l.clear();
}

TEST(TimeoutList, Reinsert)
{
    TimeoutList l;

    Neighbor n1;
    l.insert(n1, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());

    Neighbor n2;
    l.insert(n2, 10);
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());

    l.insert(n1, 12);
    ASSERT_EQ(l.iterator_to(n2), l.begin());
    ASSERT_EQ(l.iterator_to(n1), ++l.begin());

    l.insert(n1, 10);
    ASSERT_EQ(l.iterator_to(n2), l.begin());
    ASSERT_EQ(l.iterator_to(n1), ++l.begin());

    l.insert(n1, 9);
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());

    l.insert(n1, 10);
    ASSERT_EQ(l.iterator_to(n2), l.begin());
    ASSERT_EQ(l.iterator_to(n1), ++l.begin());

    l.clear();
}

TEST(TimeoutList, EraseElement)
{
    TimeoutList l;

    Neighbor n1;
    Neighbor n2;
    Neighbor n3;
    l.insert(n2, 11);
    l.insert(n1, 10);
    l.insert(n3, 12);

    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n2), ++l.begin());
    ASSERT_EQ(l.iterator_to(n3), ++ ++l.begin());

    l.erase(l.iterator_to(n2));
    ASSERT_EQ(l.iterator_to(n1), l.begin());
    ASSERT_EQ(l.iterator_to(n3), ++l.begin());

    l.erase(l.iterator_to(n1));
    ASSERT_EQ(l.iterator_to(n3), l.begin());

    l.erase(l.iterator_to(n3));
    ASSERT_EQ(true, l.empty());
}
