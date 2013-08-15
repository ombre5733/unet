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

namespace osl
{

namespace detail
{

// This struct is a helper for casting durations. Generally, we seek to convert
// from (f * fN / fD) to (t * tN / tD), where f and t are the ticks of a
// duration and (fN / fD) and (tN / tD) are the associated periods.
// The general solution is t = f * (fN / fD) / (tN / tD).
// We can compute the ration R := rN / rD = (fN / fD) / (tN / tD) at compile
// time (after all the duration's period is a compile-time constant). To avoid
// further useless computations, we can optimize the conversion for the
// following special cases:
// 1) R = 1: The two periods are equal and we only need to cast the ticks.
// 2) R = rN / 1 with rN != 1: The from-ticks have to be multiplied with rN.
// 3) R = 1 / rD with rD != 1: The from-ticks have to be divided by rD.
template <typename FromDurationT, typename ToDurationT, typename RatioT,
          bool RatioNumeratorEqualsOne, bool RatioDenominatorEqualsOne>
struct duration_cast_helper;

// Special case R = 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, true>
{
    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        return ToDurationT(static_cast<typename ToDurationT::rep>(from.count()));
    }
};

// Special case R = rN / 1, rN != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, true, false>
{
    BOOST_CONSTEXPR ToDuration operator()(const FromDuration& fd) const
            {
                typedef typename common_type<
                  typename ToDuration::rep,
                  typename FromDuration::rep,
                  boost::intmax_t>::type C;
                return ToDuration(static_cast<typename ToDuration::rep>(
                                  static_cast<C>(fd.count()) * static_cast<C>(Period::num)));
            }

    BOOST_CONSTEXPR ToDurationT cast(const FromDurationT& from) const
    {
        ???
    }
};

// Special case R = 1 / rD, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, true>
{
};

// General case R = rN / rD, rN != 1, rD != 1.
template <typename FromDurationT, typename ToDurationT, typename RatioT>
struct duration_cast_helper<FromDurationT, ToDurationT, RatioT, false, false>
{
};

} // namespace detail

template <typename FromDurationT, typename ToDurationT>
struct duration_cast<FromDurationT, ToDurationT>
{
    detail::duration_cast_helper<???>
};

} // namespace osl

#endif // OSL_CMSIS_DURATION_HPP
