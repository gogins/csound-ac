# macOS release configuration for building Csound.
#
# This file intentionally forces standard root-level install defaults:
#   - executables under /usr/local/bin
#   - libraries under /usr/local/lib
#   - headers under /usr/local/include
#
# Note: framework or plugin install destinations that are hard-coded elsewhere
# in the Csound build system using absolute paths will not be overridden by
# these variables alone.
#
# USAGE: Create a symbolic link to this file in $HOME/cmake/.

include(GNUInstallDirs)

set(CMAKE_BUILD_TYPE "Release")
set(BUILD_STATIC_LIBRARY ON)
set(BUILD_TESTS ON)

set(BUILD_CSBEATS ON)
set(BUILD_DSSI_OPCODES OFF)
set(BUILD_MULTI_CORE ON)
set(BUILD_OSC_OPCODES ON)
set(BUILD_PADSYNTH_OPCODES ON)
set(BUILD_SCANSYN_OPCODES ON)
set(BUILD_UTILITIES ON)
set(INSTALL_PYTHON_INTERFACE OFF)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(BUILD_RELEASE OFF)

set(USE_ALSA 0)
set(USE_ATOMIC_BUILTIN 1)
set(USE_AUDIOUNIT 1)
set(USE_CURL 1)
set(USE_COREMIDI 1)
set(USE_GETTEXT 0)
set(USE_IPMIDI 1)
set(USE_JACK 1)
set(USE_LIB64 1)
set(USE_LRINT 1)
set(USE_MP3 1)
set(USE_PORTAUDIO 1)
set(USE_PORTMIDI 1)
set(USE_PULSEAUDIO 0)
set(USE_SYSTEM_PORTSMF 1)
set(USE_PIPEWIRE 0)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_16BIT_TYPE "unsigned short")
set(FAIL_MISSING ON) # Enable when packaging

# Force standard macOS root-level install locations.
set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install prefix" FORCE)
set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "User executables" FORCE)
set(CMAKE_INSTALL_SBINDIR "sbin" CACHE STRING "System admin executables" FORCE)
set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "Libraries" FORCE)
set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "Header files" FORCE)
set(CMAKE_INSTALL_DATAROOTDIR "share" CACHE STRING "Read-only architecture-independent data root" FORCE)
set(CMAKE_INSTALL_DATADIR "share" CACHE STRING "Read-only architecture-independent data" FORCE)
set(CMAKE_INSTALL_MANDIR "share/man" CACHE STRING "Manual pages" FORCE)
set(CMAKE_INSTALL_DOCDIR "share/doc/csound" CACHE STRING "Documentation" FORCE)

# Keep installed binaries and shared libraries looking in standard root-level
# locations first.
set(CMAKE_INSTALL_RPATH "/usr/local/lib;/Library/Frameworks" CACHE STRING "Install RPATH" FORCE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF CACHE BOOL "Build with install RPATH" FORCE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON CACHE BOOL "Append link paths to install RPATH" FORCE)
