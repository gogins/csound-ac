cmake_minimum_required(VERSION 3.5)
project(csound-ac)

message("CMAKE BUILD SYSTEM FOR CSOUND-AC")
message("For a fresh build, run 'bash fresh-build-linux.sh'")
include("Custom.cmake" OPTIONAL RESULT_VARIABLE CSOUND_AC_CUSTOM_CONFIGURATION)
message("CSOUND_AC_CUSTOM_CONFIGURATION: ${CSOUND_AC_CUSTOM_CONFIGURATION}")

# A certain amount of the following has been copied from the core Csound project,
# and hoisted up here to enable paths etc. to be set for all projects here.

# Top-level CMake configuration.

include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wno-sign-compare -Wno-unknown-pragmas -Wno-misleading-indentation")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "csound-ac")
add_definitions(-Dlinux)

set(APIVERSION "6.0")
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/local-linux")
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

# All dependencies are to be located from here.

# Local TARGETS are not PACKAGES!
# Locally installed packages must be searched locally first!
# Locally built packages must be searched locally first!

# First we locate all system packages, these are used to make the Ubuntu
# csound-ac package, even though local packages may be used for other
# targets.
message("CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")
set(CMAKE_PREFIX_PATH /usr/local)
find_package(Csound REQUIRED)
message("CSOUND_LIBRARIES: ${CSOUND_LIBRARIES}")
find_library(LIBSNDFILE_LIBRARY sndfile REQUIRED)
find_library(CSND6_LIBRARY csnd6 REQUIRED)
#find_package(Eigen3 REQUIRED CONFIG)
# checking property of the target
#get_target_property(inc_dir Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
#message("[DEBUG] inc_dir: ${inc_dir}")
# or checking the Eigen variable
message("[DEBUG] EIGEN3_INCLUDE_DIRS: ${EIGEN3_INCLUDE_DIRS}")
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED)
message("[DEBUG] Boost_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
message("[DEBUG] Boost_COMPONENTS: ${Boost_COMPONENTS}")
#find_package(PORTSMF)
find_package(Python3 COMPONENTS Interpreter Development)
find_package(SWIG)
include(UseSWIG)
find_package(SWIG)

###include_directories(".")
include_directories("${CSOUND_INCLUDE_DIR}")
include_directories("${CSOUND_INCLUDE_DIR}/csound")
include_directories("${CSOUND_SOURCE_HOME}/interfaces")
if (MUSICXML_FOUND)
    include_directories("${MUSICXML_INCLUDE_DIRS}")
endif()

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
        while( ${i} LESS ${ARGC} )
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

add_custom_target(doxygen_reference
    ALL
    COMMAND doxygen
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/doc")

add_subdirectory(dependencies/libmusicxml/build)
add_subdirectory(dependencies/portsmf)
add_subdirectory(CsoundAC)

# These are the real targets: the packages.

set(CPACK_COMPONENTS_ALL csound-ac)
set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE "OFF")
set(CPACK_GENERATOR TGZ)
set(CPACK_PACKAGE_CONTACT "Michael Gogins <michael.gogins@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_NAME "csound-ac")
set(CPACK_PACKAGE_VERSION "0.5.0")

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/copyright.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")

install(FILES ${CMAKE_SOURCE_DIR}/copyright.txt
    DESTINATION share/doc/${CPACK_PACKAGE_NAME}
    RENAME copyright)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/doc/html
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME})

# And all the README.md files, renamed to not conflict...

install(FILES "README.md"
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME})
install(FILES "REALTIME-AUDIO.md"
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME})
install(FILES "silencio/patches/README.md"
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME}
    RENAME "README-CSOUND-PATCHES.md")
install(FILES "silencio/README.md"
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME}
    RENAME "README-SILENCIO.md")
install(FILES "playpen/README.md"
    DESTINATION share/doc/${CPACK_DEBIAN_PACKAGE_NAME}
    RENAME "README-PLAYPEN.md")

include (CPack)
