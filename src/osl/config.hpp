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

#ifndef OSL_CONFIG_HPP
#define OSL_CONFIG_HPP

// ----=====================================================================----
//     Keil CMSIS
// ----=====================================================================----

#define OSL_IMPLEMENTATION_CXX11
#if defined(OSL_IMPLEMENTATION_CXX11)
#  define CMSIS_SYSTICK_FREQUENCY   100000
#endif // OSL_IMPLEMENTATION_CXX11

// ----=====================================================================----
//     C++11
// ----=====================================================================----

// #define OSL_IMPLEMENTATION_KEIL_CMSIS

#endif // OSL_CONFIG_HPP
