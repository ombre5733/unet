/*****************************************************************************
**
** OS abstraction layer for ARM's CMSIS.
** Copyright (C) 2013  Manuel Freiberger
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see http://www.gnu.org/licenses/gpl-3.0.
**
*****************************************************************************/

#ifndef OSL_CMSIS_DURATION_HPP
#define OSL_CMSIS_DURATION_HPP

#include <boost/config.hpp>
#include <boost/ratio.hpp>
#include <boost/type_traits/common_type.hpp>

#include <cstdint>
#include <limits>

namespace osl
{
namespace chrono
{

// ----=====================================================================----
//     treat_as_floating_point
// ----=====================================================================----

//template <class RepT>
//struct treat_as_floating_point : std::is_floating_point<RepT> {};

// ----=====================================================================----
//     duration_values
// ----=====================================================================----

//! Create special tick values for a duration.
template <typename RepT>
struct duration_values
{
    static BOOST_CONSTEXPR RepT zero()
    {
        return RepT(0);
    }

    static BOOST_CONSTEXPR RepT min()
    {
        return std::numeric_limits<RepT>::lowest();
    }

    static BOOST_CONSTEXPR RepT max()
    {
        return std::numeric_limits<RepT>::max();
    }
};

// ----=====================================================================----
//     duration
// ----=====================================================================----

//! A duration of time.
//! A duration measures an amount of time. It is defined by a number of ticks
//! and a period which is the time between two ticks in seconds.
template <typename RepT, typename PeriodT = boost::ratio<1> >
class duration
{
public:
    typedef RepT rep;
    typedef PeriodT period;

    //! Creates a duration of zero periods.
    BOOST_CONSTEXPR duration() /*= default*/
        : m_count(duration_values<rep>::zero())
    {
    }

    duration(const duration& other) /*= default*/
        : m_count(other.m_count)
    {
    }

    template <typename Rep2>
    BOOST_CONSTEXPR explicit duration(const Rep2& count,
                                      //! \todo Conversion between RepT and Rep2 is missing
                                      typename boost::enable_if_c<false>* = 0)
    {
        m_count = count;
    }

    duration& operator= (const duration& other) /*= default*/
    {
        if (this != &other)
            m_count = other.m_count;
        return *this;
    }

    //! Returns the number of ticks.
    BOOST_CONSTEXPR rep count() const
    {
        return m_count;
    }

    // Arithmetic operators.

    BOOST_CONSTEXPR duration operator+ () const
    {
        return duration(rep);
    }

    BOOST_CONSTEXPR duration operator- () const
    {
        return duration(-rep);
    }

    duration& operator++ ()
    {
        ++m_count;
        return *this;
    }

    duration operator++ (int)
    {
        return duration(m_count++);
    }

    duration& operator-- ()
    {
        --m_count;
        return *this;
    }

    duration operator-- (int)
    {
        return duration(m_count--);
    }

    //! Adds another duration.
    //! Adds the \p other duration to this duration and returns this duration.
    duration& operator+= (const duration& other)
    {
        m_count += other.m_count;
        return *this;
    }

    //! Subtracts another duration.
    //! Subtracts the \p other duration from this duration and returns this
    //! duration.
    duration& operator-= (const duration& other)
    {
        m_count -= other.m_count;
        return *this;
    }

    // Special values.

    static BOOST_CONSTEXPR duration zero()
    {
        return duration(duration_values<rep>::zero());
    }

    static BOOST_CONSTEXPR duration min()
    {
        return duration(duration_values<rep>::min());
    }

    static BOOST_CONSTEXPR duration max()
    {
        return duration(duration_values<rep>::max());
    }

private:
    rep m_count;
};

typedef duration<int32_t, boost::micro> microseconds;
typedef duration<int32_t, boost::milli> milliseconds;
typedef duration<int32_t> seconds;
typedef duration<int32_t, boost::ratio<60> > minutes;

// ----=====================================================================----
//     duration_cast
// ----=====================================================================----

namespace detail
{

// A trait to determine if a type is a duration. We need this for duration_cast.
template <typename TypeT>
struct is_duration : boost::false_type
{
};

template <typename RepT, typename PeriodT>
struct is_duration<duration<RepT, PeriodT> > : boost::true_type
{
};

// This struct is a helper for casting durations. Generally, we seek to convert
// from (f * fN / fD) to (t * tN / tD), where f and t are the ticks of a
// duration and (fN / fD) and (tN / tD) are the associated periods.
// The general solution is t = f * (fN / fD) / (tN / tD).
// We can compute the ration R := rN / rD = (fN / fD) / (tN / tD) at compile
// time (after all the duration's period is a compile-time constant). However,
// the standard requires to avoid useless computations e.g. multiplications or
// divisions by 1. Thus, this template is specialized for the following cases:
// 1) R = 1: The two periods are equal and we only need to cast the ticks.
// 2) R = rN / 1 with rN != 1: The from-ticks have to be multiplied with rN.
// 3) R = 1 / rD with rD != 1: The from-ticks have to be divided by rD.
//
// Note: This implementation differs from the standard as it does not perform
// the computations in the widest type available but in the fastest. To be
// more precise, the computation is done in the common type between the
// input, output and the fastest integer type. The outcome is then converted
// to the result type.
template <typename FromDurationT, typename ToDurationT, typename RatioT,
          bool RatioNumeratorEqualsOne, bool RatioDenominatorEqualsOne>
struct duration_cast_helper;

// Special case R = 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, true>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               from.count()));
    }
};

// Special case R = rN / 1, rN != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, true>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                fastest_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)));
    }
};

// Special case R = 1 / rD, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, false>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                fastest_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               / static_cast<common_type>(RatioT::den)));
    }
};

// General case R = rN / rD, rN != 1, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, false>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        typedef typename boost::common_type<
                typename FromDurationT::rep,
                typename ToDurationT::rep,
                fastest_int_type>::type common_type;

        return ToDurationT(static_cast<typename ToDurationT::rep>(
                               static_cast<common_type>(from.count())
                               * static_cast<common_type>(RatioT::num)
                               / static_cast<common_type>(RatioT::den)));
    }
};

template <typename FromDurationT, typename ToDurationT>
struct duration_caster
{
    typedef typename boost::ratio_divide<
                         typename FromDurationT::period,
                         typename ToDurationT::period>::type ratio;

    typedef duration_cast_helper<FromDurationT, ToDurationT,
                                 ratio,
                                 ratio::num == 1,
                                 ratio::den == 1> helper;

    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        return helper().cast(from);
    }
};

} // namespace detail

//! A utility function to cast durations.
//! Cast from a <tt>duration<RepT, PeriodT></tt> given in \p d to another
//! duration templated by \p ToDuration.
template <typename ToDurationT, typename RepT, typename PeriodT>
BOOST_CONSTEXPR
typename boost::enable_if<detail::is_duration<ToDurationT>, ToDurationT>::type
duration_cast(const duration<RepT, PeriodT>& d)
{
    return detail::duration_caster<duration<RepT, PeriodT>,
                                   ToDurationT>().cast(d);
}

} // namespace chrono
} // namespace osl

#endif // OSL_CMSIS_DURATION_HPP
