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
namespace csound {
    
#if defined(__APPLE__) && defined(__MACH__)
#  if defined(__clang__)
#    pragma message "Platform.hpp: macOS with Clang." 
#    if !defined(SWIG)
#      define SILENCE_PUBLIC //__attribute__ ( (visibility("default")) )
#    else
#      define SILENCE_PUBLIC
#    endif
#  endif
#elif defined(_WIN32)
#  if defined(_MSC_VER)
#    pragma message "Platform.hpp: Windows with MSVC." 
#    define _USE_MATH_DEFINES
#    if !defined(PATH_MAX)
#      define PATH_MAX _MAX_PATH
#    endif
#    define _CRT_SECURE_NO_WARNINGS
#    if !defined(SWIG)
#      define SILENCE_PUBLIC __declspec(dllexport)
#    else
#      define SILENCE_PUBLIC 
#    endif
#  endif
#elif defined(__linux__)
#  if defined(__GNUC__)
#    pragma message "Platform.hpp: Linux with gcc." 
#    define SILENCE_PUBLIC __attribute__ ( (visibility("default")) )
#  endif
#else
#  define SILENCE_PUBLIC
#endif

};

#endif

