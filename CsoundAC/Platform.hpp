#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED
/*
* C S O U N D
*
* L I C E N S E
*
* This software is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This software is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this software; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if (defined(WIN32) || defined(_WIN32)) && !defined(SWIG) && !defined(_MSC_VER)
#  define SILENCE_PUBLIC __declspec(dllexport)
#  define _USE_MATH_DEFINES
#elif defined(__GNUC__) && (__GNUC__ >= 4) /* && !defined(__MACH__) */
#  define SILENCE_PUBLIC        __attribute__ ( (visibility("default")) )
#else
#  define SILENCE_PUBLIC
#endif

#endif

