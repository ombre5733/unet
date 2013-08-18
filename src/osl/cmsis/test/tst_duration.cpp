#include "../duration.hpp"

#include "gtest/gtest.h"

TEST(Seconds, DefaultConstructor)
{
    osl::chrono::seconds s;
    ASSERT_EQ(0, s.count());
}

TEST(Seconds, ConstructorWithArgument)
{
    osl::chrono::seconds s(42);
    ASSERT_EQ(42, s.count());
}

TEST(Seconds, CopyConstructor)
{
    osl::chrono::seconds s1(42);
    osl::chrono::seconds s2(ms1);
    ASSERT_EQ(42, s2.count());
}

TEST(Seconds, Zero)
{
    osl::chrono::seconds s = osl::chrono::seconds::zero();
    ASSERT_EQ(0, s.count());
}

TEST(Seconds, Min)
{
    osl::chrono::seconds s = osl::chrono::seconds::min();
    ASSERT_EQ(-2147483648, s.count());
}

TEST(Seconds, Max)
{
    osl::chrono::seconds s = osl::chrono::seconds::max();
    ASSERT_EQ(2147483647, s.count());
}


TEST(duration_cast, s_to_ms)
{
    osl::chrono::seconds s(23);
    osl::chrono::milliseconds ms = osl::chrono::duration_cast<osl::chrono::milliseconds>(s);
    ASSERT_EQ(23000, ms.count());
}

TEST(duration_cast, s_to_us)
{
    osl::chrono::seconds s(23);
    osl::chrono::microseconds us = osl::chrono::duration_cast<osl::chrono::microseconds>(s);
    ASSERT_EQ(23000000, us.count());
}

TEST(duration_cast, min_to_s)
{
    osl::chrono::minutes min(23);
    osl::chrono::seconds s = osl::chrono::duration_cast<osl::chrono::seconds>(min);
    ASSERT_EQ(23*60, s.count());
}

TEST(duration_cast, min_to_ms)
{
    osl::chrono::minutes min(23);
    osl::chrono::milliseconds ms = osl::chrono::duration_cast<osl::chrono::milliseconds>(min);
    ASSERT_EQ(23*60000, ms.count());
}

TEST(duration_cast, min_to_us)
{
    osl::chrono::minutes min(23);
    osl::chrono::milliseconds us = osl::chrono::duration_cast<osl::chrono::microseconds>(min);
    ASSERT_EQ(23*60000000, us.count());
}

double slept_s = 0;
double slept_ms = 0;
double slept_us = 0;

template <typename RepT>
void sleep(osl::chrono::duration<RepT, boost::ratio<1> >& d)
{
    slept_s = d.count();
}

template <typename RepT>
void sleep(osl::chrono::duration<RepT, boost::milli>& d)
{
    slept_ms = d.count();
}

template <typename RepT>
void sleep(osl::chrono::duration<RepT, boost::micro>& d)
{
    slept_us = d.count();
}

TEST(duration, OverloadResolution)
{
    sleep(osl::chrono::seconds(11));
    ASSERT_EQ(11, slept_s);
    slept_s = 0;
    sleep(osl::chrono::duration<uint8_t, boost::ratio<1> >(23));
    ASSERT_EQ(23, slept_s);
    slept_s = 0;
    sleep(osl::chrono::duration<double, boost::ratio<1> >(3.14));
    ASSERT_EQ(3.14, slept_s);
    slept_s = 0;

    sleep(osl::chrono::milliseconds(11));
    ASSERT_EQ(11, slept_ms);
    slept_ms = 0;
    sleep(osl::chrono::duration<uint8_t, boost::milli>(23));
    ASSERT_EQ(23, slept_ms);
    slept_ms = 0;
    sleep(osl::chrono::duration<double, boost::milli>(3.14));
    ASSERT_EQ(3.14, slept_ms);
    slept_ms = 0;

    sleep(osl::chrono::microseconds(11));
    ASSERT_EQ(11, slept_us);
    slept_us = 0;
    sleep(osl::chrono::duration<uint8_t, boost::micro>(23));
    ASSERT_EQ(23, slept_us);
    slept_us = 0;
    sleep(osl::chrono::duration<double, boost::micro>(3.14));
    ASSERT_EQ(3.14, slept_us);
    slept_us = 0;
}
