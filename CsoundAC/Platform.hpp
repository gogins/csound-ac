#pragma once
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
#ifndef PLATFORM_HPP_INCLUDED
#define PLATFORM_HPP_INCLUDED

#if defined(_WIN32)
#    if !defined(_USE_MATH_DEFINES)
#        define _USE_MATH_DEFINES
#    endif
#    if !defined(PATH_MAX) && defined(_MAX_PATH)
#        define PATH_MAX _MAX_PATH
#    endif
#    if !defined(_CRT_SECURE_NO_WARNINGS)
#        define _CRT_SECURE_NO_WARNINGS
#    endif
#endif
/*
    SILENCE_PUBLIC controls symbol visibility/export.

    Cases:

        SWIG:
            No export/import attributes.

        Static library:
            No export/import attributes.

        Windows shared library:
            __declspec(dllexport) when building the DLL.
            __declspec(dllimport) when consuming the DLL.

        macOS/Linux/Emscripten:
            Use ELF/Mach-O visibility attributes.
*/
#if defined(SWIG)
#    define SILENCE_PUBLIC
#elif defined(CSOUNDAC_STATIC)
#    define SILENCE_PUBLIC
#elif defined(_WIN32)
#    if defined(CSOUNDAC_BUILDING_LIBRARY)
#        define SILENCE_PUBLIC __declspec(dllexport)
#    else
#        define SILENCE_PUBLIC __declspec(dllimport)
#    endif
#elif defined(__GNUC__) || defined(__clang__)
#    define SILENCE_PUBLIC __attribute__((visibility("default")))
#else
#    define SILENCE_PUBLIC
#endif
#endif
