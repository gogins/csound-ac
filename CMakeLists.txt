cmake_minimum_required(VERSION 3.28+)
project(csound-ac)

find_package(Doxygen REQUIRED)

message("CMAKE BUILD SYSTEM FOR CSOUND-AC")
message("For a fresh build, run 'bash fresh-build-linux.sh'")
include("Custom.cmake" OPTIONAL RESULT_VARIABLE CSOUND_AC_CUSTOM_CONFIGURATION)
message("CSOUND_AC_CUSTOM_CONFIGURATION: ${CSOUND_AC_CUSTOM_CONFIGURATION}")

# A certain amount of the following has been copied from the core Csound project,
# and hoisted up here to enable paths etc. to be set for all projects here.

# Top-level CMake configuration.

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    add_compile_options(/WX-)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-deprecated-declarations -Wno-sign-compare -Wno-unknown-pragmas -Wno-misleading-indentation -Wno-writable-strings")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    # add_compile_options(-Wno-c++11-narrowing)
endif()


# CMAKE_SYSTEM_NAME relevant here can be Darwin, Windows, Linux, or Android.
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(GENERATE_DOCUMENTATION ON CACHE BOOL "Enable building documentation.")
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "csound-ac")
if(DARWIN)
    set(CMAKE_MACOSX_RPATH ON)
    set(CMAKE_INSTALL_RPATH "/opt/homebrew/lib;/usr/local/lib;~/.local/lib;")
    set(CMAKE_PREFIX_PATH "/opt/homebrew;")
else()
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/local-linux")
endif()
add_definitions(-Dlinux)

set(APIVERSION "7.1")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/dependencies/csound/cmake/Modules")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")
set(CMAKE_BUILD_TYPE "RelWithDebInfo")
set(ALLOW_DUPLICATE_CUSTOM_TARGETS "Yes")
add_definitions(-DUSE_DOUBLE)
set(USE_DOUBLE "Yes")
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(BUILD_PLUGINS_DIR ${BUILD_DIR})
set(BUILD_BIN_DIR ${BUILD_DIR})
set(BUILD_LIB_DIR ${BUILD_DIR})
set(EXECUTABLE_INSTALL_DIR "bin")
option(USE_LIB64 "Set to on to set installation directory for libraries to lib64" OFF)
if(USE_LIB64)
    set(LIBRARY_INSTALL_DIR "lib64")
    add_definitions("-DLIB64")
else()
    set(LIBRARY_INSTALL_DIR "lib")
endif()
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CPACK_PACKAGING_INSTALL_PREFIX: ${CPACK_PACKAGING_INSTALL_PREFIX}")
message(STATUS "BUILD_BIN_DIR: ${BUILD_BIN_DIR}")
message(STATUS "BUILD_LIB_DIR: ${BUILD_LIB_DIR}")
message(STATUS "LIBRARY INSTALL DIR: ${LIBRARY_INSTALL_DIR}")
message(STATUS "PLUGIN_INSTALL_DIR: ${PLUGIN_INSTALL_DIR}")
option(USE_LRINT "Use lrint/lrintf for converting floating point values to integers." ON)
option(USE_CURL "Use CURL library" ON)

# ------------------------------------------------------------------------------
# Dependency policy
#
# - Csound is assumed to be canonically installed and is NOT searched locally.
# - For all other dependencies, if headers/libs exist in the repo or the current
#   build tree, those are preferred and used in preference to any system copies.
# ------------------------------------------------------------------------------

option(CSOUND_AC_PREFER_LOCAL_DEPS "Prefer dependencies from the repo/build tree when available (except Csound)." ON)

set(CSOUND_AC_LOCAL_PREFIXES
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_BINARY_DIR}/local"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/local"
    "${CMAKE_SOURCE_DIR}/local-linux"
    "${CMAKE_SOURCE_DIR}/dependencies"
)

# Make CMake package discovery prefer local prefixes (except where NO_DEFAULT_PATH is used).
if(CSOUND_AC_PREFER_LOCAL_DEPS)
    foreach(prefix IN LISTS CSOUND_AC_LOCAL_PREFIXES)
        if(EXISTS "${prefix}")
            list(PREPEND CMAKE_PREFIX_PATH "${prefix}")
        endif()
    endforeach()
endif()

# Make pkg-config prefer local .pc files first.
if(CSOUND_AC_PREFER_LOCAL_DEPS)
    set(_pc_paths "")
    foreach(prefix IN LISTS CSOUND_AC_LOCAL_PREFIXES)
        list(APPEND _pc_paths
            "${prefix}/lib/pkgconfig"
            "${prefix}/lib64/pkgconfig"
            "${prefix}/share/pkgconfig"
        )
    endforeach()
    list(JOIN _pc_paths ":" _pc_paths_joined)
    if(DEFINED ENV{PKG_CONFIG_PATH} AND NOT "$ENV{PKG_CONFIG_PATH}" STREQUAL "")
        set(ENV{PKG_CONFIG_PATH} "${_pc_paths_joined}:$ENV{PKG_CONFIG_PATH}")
    else()
        set(ENV{PKG_CONFIG_PATH} "${_pc_paths_joined}")
    endif()
endif()

message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# ------------------------------------------------------------------------------
# Canonical Csound (hardcoded location preserved)
# ------------------------------------------------------------------------------

# Keep this as a canonical installed Csound; do not attempt to pick up local builds.
find_package(Csound REQUIRED)
message("CSOUND_LIBRARIES: ${CSOUND_LIBRARIES}")
message("CSOUND_INCLUDE_DIRS: ${CSOUND_INCLUDE_DIRS}")
message("CSOUND_INCLUDE_PATH: ${CSOUND_INCLUDE_PATH}")

# SHOULD find: CsoundLib64.framework/Versions/6.0/Headers/
include_directories("/Library/Frameworks/CsoundLib64.framework/Versions/6.0/Headers/")

# ------------------------------------------------------------------------------
# Other dependencies (prefer local copies if present)
# ------------------------------------------------------------------------------

find_package(PkgConfig REQUIRED)

# libsndfile: prefer locally provided headers/libs if present.
set(LIBSNDFILE_INCLUDE_DIR "")
set(LIBSNDFILE_LIBRARY "")

if(CSOUND_AC_PREFER_LOCAL_DEPS)
    set(_sndfile_include_candidates
        "${CMAKE_SOURCE_DIR}/dependencies/libsndfile/include"
        "${CMAKE_SOURCE_DIR}/dependencies/sndfile/include"
        "${CMAKE_BINARY_DIR}/dependencies/libsndfile/include"
        "${CMAKE_BINARY_DIR}/dependencies/sndfile/include"
        "${CMAKE_SOURCE_DIR}/local/include"
        "${CMAKE_BINARY_DIR}/local/include"
    )
    foreach(dir IN LISTS _sndfile_include_candidates)
        if(EXISTS "${dir}/sndfile.h")
            set(LIBSNDFILE_INCLUDE_DIR "${dir}")
            break()
        endif()
    endforeach()

    set(_sndfile_lib_candidates
        "${CMAKE_SOURCE_DIR}/dependencies/libsndfile/lib"
        "${CMAKE_SOURCE_DIR}/dependencies/libsndfile/lib64"
        "${CMAKE_BINARY_DIR}/dependencies/libsndfile/lib"
        "${CMAKE_BINARY_DIR}/dependencies/libsndfile/lib64"
        "${CMAKE_SOURCE_DIR}/local/lib"
        "${CMAKE_SOURCE_DIR}/local/lib64"
        "${CMAKE_BINARY_DIR}/local/lib"
        "${CMAKE_BINARY_DIR}/local/lib64"
    )
    foreach(dir IN LISTS _sndfile_lib_candidates)
        if(EXISTS "${dir}/libsndfile.a")
            set(LIBSNDFILE_LIBRARY "${dir}/libsndfile.a")
            break()
        endif()
        if(EXISTS "${dir}/libsndfile.dylib")
            set(LIBSNDFILE_LIBRARY "${dir}/libsndfile.dylib")
            break()
        endif()
        if(EXISTS "${dir}/libsndfile.so")
            set(LIBSNDFILE_LIBRARY "${dir}/libsndfile.so")
            break()
        endif()
    endforeach()
endif()

if(LIBSNDFILE_INCLUDE_DIR AND LIBSNDFILE_LIBRARY)
    message(STATUS "[LOCAL] Using libsndfile from:")
    message(STATUS "  LIBSNDFILE_INCLUDE_DIR: ${LIBSNDFILE_INCLUDE_DIR}")
    message(STATUS "  LIBSNDFILE_LIBRARY:     ${LIBSNDFILE_LIBRARY}")
else()
    pkg_check_modules(SNDFILE REQUIRED sndfile)

    # Preserve original variable names used by subprojects, but ensure we end up
    # with a real library *path* (not just 'sndfile' / '-lsndfile'), so link lines work on macOS.
    set(LIBSNDFILE_INCLUDE_DIR "${SNDFILE_INCLUDE_DIRS}")
    set(LIBSNDFILE_LIBRARY_DIRS "${SNDFILE_LIBRARY_DIRS}")

    # Homebrew sometimes returns "sndfile" (not "-lsndfile", not a path).
    # Resolve explicitly using the directories we know contain the actual files.
    set(LIBSNDFILE_LIBRARY "")
    set(_libsndfile_resolved "")

    # Add stable Homebrew paths as additional fallbacks.
    list(APPEND LIBSNDFILE_LIBRARY_DIRS
        "/opt/homebrew/opt/libsndfile/lib"
        "/opt/homebrew/lib"
    )

    # First: explicit filename checks (works with your actual Homebrew layout).
    foreach(_d IN LISTS LIBSNDFILE_LIBRARY_DIRS)
        if(EXISTS "${_d}/libsndfile.dylib")
            set(_libsndfile_resolved "${_d}/libsndfile.dylib")
            break()
        endif()
        if(EXISTS "${_d}/libsndfile.1.dylib")
            set(_libsndfile_resolved "${_d}/libsndfile.1.dylib")
            break()
        endif()
        if(EXISTS "${_d}/libsndfile.a")
            set(_libsndfile_resolved "${_d}/libsndfile.a")
            break()
        endif()
    endforeach()

    # Second: find_library fallback.
    if(NOT _libsndfile_resolved)
        find_library(_libsndfile_resolved
            NAMES sndfile libsndfile
            PATHS ${LIBSNDFILE_LIBRARY_DIRS}
        )
    endif()

    if(_libsndfile_resolved)
        set(LIBSNDFILE_LIBRARY "${_libsndfile_resolved}")
    else()
        message(FATAL_ERROR
            "libsndfile not found as a file path. "
            "pkg-config libs='${SNDFILE_LIBRARIES}', libdirs='${SNDFILE_LIBRARY_DIRS}'. "
            "Tried also /opt/homebrew/opt/libsndfile/lib and /opt/homebrew/lib."
        )
    endif()

    message(STATUS "[PKG-CONFIG] Using libsndfile from:")
    message(STATUS "  LIBSNDFILE_INCLUDE_DIR: ${LIBSNDFILE_INCLUDE_DIR}")
    message(STATUS "  LIBSNDFILE_LIBRARY:     ${LIBSNDFILE_LIBRARY}")
    message(STATUS "  LIBSNDFILE_LIBRARY_DIRS:${LIBSNDFILE_LIBRARY_DIRS}")
endif()

set(EIGEN3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/dependencies/eigen")
message("[DEBUG] EIGEN3_INCLUDE_DIRS: ${EIGEN3_INCLUDE_DIRS}")

# Boost: prefer local prefix if headers exist there; otherwise use normal search.
if(CSOUND_AC_PREFER_LOCAL_DEPS)
    set(_boost_root "")
    foreach(prefix IN LISTS CSOUND_AC_LOCAL_PREFIXES)
        if(EXISTS "${prefix}/include/boost")
            set(_boost_root "${prefix}")
            break()
        endif()
    endforeach()

    if(_boost_root)
        message(STATUS "[LOCAL] Using Boost from prefix: ${_boost_root}")
        set(BOOST_ROOT "${_boost_root}")
        set(Boost_NO_SYSTEM_PATHS ON)
    endif()
endif()

find_package(Boost 1.65 REQUIRED)
message("[DEBUG] Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")

find_package(Python3 3.12 EXACT COMPONENTS Interpreter Development)
find_package(SWIG)
include(UseSWIG)
find_package(SWIG)

if(LIBSNDFILE_INCLUDE_DIR)
    include_directories("${LIBSNDFILE_INCLUDE_DIR}")
endif()

# Boost conflicts with GitHub's Windows runner, which #defines
# both `WIN32` and `linux`.
if(UNISTD_WIN32_INCLUDE_DIR)
    include_directories("${UNISTD_WIN32_INCLUDE_DIR}")
endif()

include_directories("${CSOUND_INCLUDE_DIR}")
include_directories("${CSOUND_INCLUDE_DIR}/csound")
include_directories("${CSOUND_SOURCE_HOME}/interfaces")

# Copied from dependencies/csound/CMakeLists.txt:

# Checks if dependencies for an enabled target are fulfilled.
# If FAIL_MISSING is true and the dependencies are not fulfilled,
# it will abort the cmake run.
# If FAIL_MISSING is false, it will set the option to OFF.
# If the target is not enabled, it will do nothing.
# example: check_deps(BUILD_NEW_PARSER FLEX_EXECUTABLE BISON_EXECUTABLE)
function(check_deps option)
    if(${option})
        set(i 1)
        while(${i} LESS ${ARGC})
            set(dep ${ARGV${i}})
            if(NOT ${dep})
                if(FAIL_MISSING)
                    message(FATAL_ERROR
                        "${option} is enabled, but ${dep}=\"${${dep}}\"")
                else()
                    message(STATUS "${dep}=\"${${dep}}\", so disabling ${option}")
                    set(${option} OFF PARENT_SCOPE)
                    # Set it in the local scope too
                    set(${option} OFF)
                endif()
            endif()
            math(EXPR i "${i}+1")
        endwhile()
    endif()
    if(${option})
        message(STATUS "${option} is enabled.")
    else()
        message(STATUS "${option} is disabled.")
    endif()
endfunction(check_deps)

add_subdirectory(dependencies/libmusicxml/build)
add_subdirectory(CsoundAC)

if(GENERATE_DOCUMENTATION)
    message("Generating documentation.")
    add_custom_target(doxygen_reference
        ALL
        COMMAND ./latex.sh
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/doc")
else()
    message("Not generating documentation.")
endif()

# These are the real targets: the packages.

set(CPACK_COMPONENTS_ALL csound-ac)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE "ON")
set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_CONTACT "Michael Gogins <michael.gogins@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_NAME "csound-ac")
set(CPACK_PACKAGE_VERSION "0.6.1")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/copyright.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

install(FILES ${CMAKE_SOURCE_DIR}/copyright.txt
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME copyright)
if(GENERATE_DOCUMENTATION)
    install(FILES ${CMAKE_SOURCE_DIR}/doc/latex/csound-ac.pdf
        DESTINATION share/doc/${CPACK_PACKAGE_NAME})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/html
        DESTINATION share/doc/${CPACK_PACKAGE_NAME})
endif()
install(DIRECTORY ${CMAKE_SOURCE_DIR}/patches
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}/patches)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/playpen
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}/playpen)
install(DIRECTORY "${CMAKE_SOURCE_DIR}/user-guide/"
    DESTINATION  share/doc/${CPACK_PACKAGE_NAME}/user-guide
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "44025833484_70440d3a59_o.jpg"
    PATTERN "*.pdf")
# And all the README.md files, renamed to not conflict...

install(FILES "README.md"
    DESTINATION .
    RENAME "README-CSOUND-AC.md")
install(FILES "README.md"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME "README-CSOUND-AC.md")
install(FILES "LICENSE"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME})
install(FILES "LICENSE"
    DESTINATION .
    RENAME "LICENSE-CSOUND-AC")
install(FILES "REALTIME-AUDIO.md"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME})
install(FILES "patches/README.md"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME "README-CSOUND-PATCHES.md")
install(FILES "silencio/README.md"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME "README-SILENCIO.md")
install(FILES "playpen/README.md"
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME "README-PLAYPEN.md")

# Make CPack stage installation into the package directory via DESTDIR,
# instead of installing into the real CMAKE_INSTALL_PREFIX during packaging.
set(CPACK_SET_DESTDIR ON)

# Package contents should be rooted at the staging area (not at /opt/homebrew).
# This makes the ZIP contain "csound-ac/..." rather than trying to write into /opt/homebrew.
set(CPACK_PACKAGING_INSTALL_PREFIX "csound-ac")

include(CPack)
