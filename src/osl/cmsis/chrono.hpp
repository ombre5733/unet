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

#ifndef OSL_CMSIS_CHRONO_HPP
#define OSL_CMSIS_CHRONO_HPP

#include "cmsis_os.h"

#include <boost/ratio.hpp>

#include <cstdint>
#include <limits>

namespace osl
{
namespace chrono
{

using boost::ratio;
using boost::milli;

// ----=====================================================================----
//     duration_values
// ----=====================================================================----

template <class RepT>
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
//     treat_as_floating_point
// ----=====================================================================----

//template <class RepT>
//struct treat_as_floating_point : std::is_floating_point<RepT> {};

// ----=====================================================================----
//     duration
// ----=====================================================================----

//! A duration of time.
//! A duration measures an amount of time. It is defined by a number of ticks
//! and a period which is the time between two ticks in seconds.
template <typename RepT, typename PeriodT>
class duration
{
public:
    typedef RepT rep;
    typedef PeriodT period;

    //! Creates a duration of zero periods.
    BOOST_CONSTEXPR duration() /*= default*/
    {
        m_count = duration_values<rep>::zero();
    }

    duration(const duration& other) /*= default*/
        : m_count(other.m_count)
    {
    }

    template <typename Rep2>
    BOOST_CONSTEXPR explicit duration(const Rep2& count)
    {
        //! \todo Conversion between Rep and Rep2 is missing
        m_count = count;
    }

    //! Returns the number of ticks.
    BOOST_CONSTEXPR rep count() const
    {
        return m_count;
    }

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

typedef duration<int32_t, milli> milliseconds;

// ----=====================================================================----
//     time_point
// ----=====================================================================----

//! A time point.
template <typename ClockT, typename DurationT = typename ClockT::duration>
class time_point
{
public:
    typedef ClockT clock;
    typedef DurationT duration;
    typedef typename duration::rep rep;
    typedef typename duration::period period;

    BOOST_CONSTEXPR time_point()
    {
    }

    BOOST_CONSTEXPR explicit time_point(const duration& d)
        : m_duration(d)
    {
    }

    duration time_since_epoch() const
    {
        return m_duration;
    }

    static BOOST_CONSTEXPR time_point max()
    {
        return time_point(duration::max());
    }

    static BOOST_CONSTEXPR time_point min()
    {
        return time_point(duration::min());
    }

private:
    duration m_duration;
};

// ----=====================================================================----
//     high_resolution_clock
// ----=====================================================================----

//! The high-resolution clock.
//! This class provides access to the system's high-resolution clock.
class high_resolution_clock
{
    typedef int32_t rep;
    typedef ratio<1, CMSIS_SYSTICK_FREQUENCY> period;
    typedef chrono::duration<rep, period> duration;
    typedef chrono::time_point<high_resolution_clock> time_point;

    static BOOST_CONSTEXPR_OR_CONST bool is_steady = false;

    static time_point now()
    {
        return time_point(duration(osKernelSysTick()));
    }
};

} // namespace chrono
} // namespace osl

#endif // OSL_CMSIS_CHRONO_HPP
