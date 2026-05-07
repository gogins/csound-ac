# Build CsoundAC
cmake_minimum_required(VERSION 3.28+)
include(CheckIncludeFiles)
include(GNUInstallDirs)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-fPIC)

set(CMAKE_INCLUDE_CURRENT_DIR "On")

option(BUILD_CSOUND_AC "Build CsoundAC extension module for Csound with algorithmic composition (needs C++ interface)" ON)
option(BUILD_CSOUND_AC_PYTHON_INTERFACE "Build the Csound AC Python interface" ON)

# Prefer local dependencies (from repo/build tree) when available.
# Canonical Csound is handled in the top-level CMakeLists.txt and is not overridden here.
option(CSOUND_AC_PREFER_LOCAL_DEPS "Prefer dependencies from the repo/build tree when available (except Csound)." ON)

set(CSOUND_AC_INSTALL_PYTHON_TO_SITEARCH_DEFAULT OFF)
if(APPLE)
    set(CSOUND_AC_INSTALL_PYTHON_TO_SITEARCH_DEFAULT ON)
endif()
option(CSOUND_AC_INSTALL_PYTHON_TO_SITEARCH
    "Install Python interface into the active interpreter site-packages in addition to the portable release layout."
    ${CSOUND_AC_INSTALL_PYTHON_TO_SITEARCH_DEFAULT}
)


find_package(SWIG)

# /opt/homebrew/Cellar/python@3.12/3.12.7_1/Frameworks/Python.framework/Versions/3.12/lib/libpython3.12.dylib -> ../Python)
find_package(Python3 3.12 EXACT COMPONENTS Interpreter Development)
message(STATUS "Python libraries:  ${PYTHONLIBS_VERSION_STRING}")

# Ensure LIBSNDFILE_LIBRARY is a resolvable library path (not just '-lsndfile').
# The top-level CMake provides LIBSNDFILE_LIBRARY_DIRS when discovered via pkg-config.
if(LIBSNDFILE_LIBRARY MATCHES "^-l.+" OR LIBSNDFILE_LIBRARY STREQUAL "sndfile" OR LIBSNDFILE_LIBRARY STREQUAL "-lsndfile")
    set(_libsndfile_resolved "")
    if(LIBSNDFILE_LIBRARY_DIRS)
        find_library(_libsndfile_resolved
            NAMES sndfile libsndfile
            HINTS ${LIBSNDFILE_LIBRARY_DIRS}
            NO_DEFAULT_PATH
        )
    endif()
    if(NOT _libsndfile_resolved)
        find_library(_libsndfile_resolved NAMES sndfile libsndfile)
    endif()
    if(_libsndfile_resolved)
        set(LIBSNDFILE_LIBRARY "${_libsndfile_resolved}")
    endif()
endif()
message(STATUS "LIBSNDFILE_LIBRARY (resolved): ${LIBSNDFILE_LIBRARY}")

# Link against the in-tree MusicXML library and explicitly bundle its runtime
# library in the csound-ac release layout. Do not run libmusicxml's own install
# rules, because those install schemas/tools outside the csound-ac layout.
set(MUSICXML2_LIBRARY "")
set(MUSICXML2_TARGET "")
set(MUSICXML2_LINK_LIBRARIES "")
set(MUSICXML2_RUNTIME_TARGET "")
set(MUSICXML2_RUNTIME_FILE "")

# Prefer a CMake target from the libmusicxml subproject. In current libmusicxml
# builds this may be a shared library target; if so, bundle that exact runtime.
foreach(tgt IN ITEMS libmusicxml2 musicxml2 libmusicxml musicxml)
    if(TARGET ${tgt})
        set(MUSICXML2_TARGET "${tgt}")
        get_target_property(_musicxml2_target_type ${tgt} TYPE)
        if(_musicxml2_target_type STREQUAL "SHARED_LIBRARY" OR
           _musicxml2_target_type STREQUAL "MODULE_LIBRARY" OR
           _musicxml2_target_type STREQUAL "UNKNOWN_LIBRARY")
            set(MUSICXML2_RUNTIME_TARGET "${tgt}")
        endif()
        break()
    endif()
endforeach()

if(MUSICXML2_TARGET)
    list(APPEND MUSICXML2_LINK_LIBRARIES ${MUSICXML2_TARGET})
else()
    # Fall back to a library file in the repo/build tree. Prefer shared runtime
    # libraries so they can be bundled; static archives are accepted only if no
    # shared runtime is available.
    set(_musicxml2_candidates
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/liblibmusicxml.dylib"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/lib/liblibmusicxml.dylib"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/liblibmusicxml.so"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/lib/liblibmusicxml.so"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/bin/libmusicxml.dll"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/bin/libmusicxml.dll"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/libmusicxml.dll"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/lib/libmusicxml.dll"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/libmusicxml2.a"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/lib/libmusicxml2.a"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/libmusicxml2.a"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/libmusicxml2.a"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/Release/libmusicxml2.lib"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/Release/libmusicxml2.lib"
        "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/libmusicxml2.lib"
        "${CMAKE_SOURCE_DIR}/dependencies/libmusicxml/build/lib/libmusicxml2.lib"
    )

    foreach(p IN LISTS _musicxml2_candidates)
        if(EXISTS "${p}")
            set(MUSICXML2_LIBRARY "${p}")
            break()
        endif()
    endforeach()

    if(NOT MUSICXML2_LIBRARY)
        if(WIN32)
            set(MUSICXML2_LIBRARY "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/Release/libmusicxml2.lib")
        else()
            set(MUSICXML2_LIBRARY "${CMAKE_BINARY_DIR}/dependencies/libmusicxml/build/lib/liblibmusicxml${CMAKE_SHARED_LIBRARY_SUFFIX}")
        endif()
    endif()

    if(MUSICXML2_LIBRARY MATCHES "\.(dylib|so|dll)$")
        set(MUSICXML2_RUNTIME_FILE "${MUSICXML2_LIBRARY}")
    endif()

    list(APPEND MUSICXML2_LINK_LIBRARIES "${MUSICXML2_LIBRARY}")
endif()

message(STATUS "MUSICXML2_TARGET:         ${MUSICXML2_TARGET}")
message(STATUS "MUSICXML2_LIBRARY:        ${MUSICXML2_LIBRARY}")
message(STATUS "MUSICXML2_RUNTIME_TARGET: ${MUSICXML2_RUNTIME_TARGET}")
message(STATUS "MUSICXML2_RUNTIME_FILE:   ${MUSICXML2_RUNTIME_FILE}")
message(STATUS "MUSICXML2_LINK_LIBRARIES: ${MUSICXML2_LINK_LIBRARIES}")

list(APPEND CMAKE_SWIG_FLAGS
    "-I${CMAKE_SOURCE_DIR}/dependencies/csound/interfaces"
)

if(BUILD_CSOUND_AC)
    set(libcsoundac_SRCS
        Cell.cpp
        ChordLindenmayer.cpp
        ChordSpace.cpp
        Composition.cpp
        Conversions.cpp
        Counterpoint.cpp
        CounterpointMain.cpp
        CounterpointNode.cpp
        CppSound.cpp
        CsoundFile.cpp
        Event.cpp
        Lindenmayer.cpp
        MCRM.cpp
        Midifile.cpp
        MusicModel.cpp
        Node.cpp
        Random.cpp
        Rescale.cpp
        Score.cpp
        ScoreModel.cpp
        ScoreNode.cpp
        Sequence.cpp
        Shell.cpp
        StrangeAttractor.cpp
        System.cpp
        Voicelead.cpp
        VoiceleadingNode.cpp
        ImageToScore.cpp
        ../dependencies/portsmf/allegro.cpp
        ../dependencies/portsmf/allegrord.cpp
        ../dependencies/portsmf/allegroserial.cpp
        ../dependencies/portsmf/allegrosmfrd.cpp
        ../dependencies/portsmf/allegrosmfwr.cpp
        ../dependencies/portsmf/allegrowr.cpp
        ../dependencies/portsmf/mfmidi.cpp
        ../dependencies/portsmf/strparse.cpp
        trace.cpp
    )
    message(STATUS "libcsoundac_SRCS: " ${libcsoundac_SRCS})

    if(MSVC)
        add_library(libCsoundAC STATIC ${libcsoundac_SRCS})
    else()
        add_library(libCsoundAC SHARED ${libcsoundac_SRCS})
    endif()

    if(WIN32)
        set(_csound_ac_windows_cimg_definitions
            cimg_display=0
            cimg_use_jpeg=0
            cimg_use_png=0
            cimg_use_tiff=0
            cimg_use_fftw3=0
            cimg_verbosity=0
            cimg_OS=2
            cimg_use_fnmatch=0
        )
    endif()

    target_compile_definitions(libCsoundAC
    PRIVATE
        USE_DOUBLE
        CSOUNDAC_BUILDING_LIBRARY
        $<$<STREQUAL:$<TARGET_PROPERTY:libCsoundAC,TYPE>,STATIC_LIBRARY>:CSOUNDAC_STATIC>

        CSOUND_FOUND=$<BOOL:${CSOUND_FOUND}>

        CSOUND_EXECUTABLE="${CSOUND_EXECUTABLE}"
        CSOUND_LIBRARY="${CSOUND_LIBRARY}"
        CSOUND_LIBRARIES="${CSOUND_LIBRARIES}"

        CSOUND_INCLUDE_DIR="${CSOUND_INCLUDE_DIR}"
        CSOUND_INCLUDE_DIRS="${CSOUND_INCLUDE_DIRS}"
        CSOUND_FRAMEWORK_DIR="${CSOUND_FRAMEWORK_DIR}"

        CSOUND_VERSION="${CSOUND_VERSION}"
        CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
    )

    # Ensure the libmusicxml build happens before we try to link against it.
    if(MUSICXML2_TARGET)
        add_dependencies(libCsoundAC ${MUSICXML2_TARGET})
    endif()

    # Always prefer headers from this source tree over installed headers.
    target_include_directories(libCsoundAC
        BEFORE PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/CsoundAC
    )

    target_include_directories(libCsoundAC SYSTEM PRIVATE ${Python3_INCLUDE_DIRS})
    target_include_directories(libCsoundAC SYSTEM PRIVATE ${CSOUND_INCLUDE_DIRS})
    target_include_directories(libCsoundAC PRIVATE ${EIGEN3_INCLUDE_DIRS})
    target_include_directories(libCsoundAC PRIVATE ${Boost_INCLUDE_DIR})
    target_include_directories(libCsoundAC PRIVATE ../dependencies/portsmf)
    target_include_directories(libCsoundAC PRIVATE ../dependencies/stb)
    target_include_directories(libCsoundAC PRIVATE ../dependencies/CImg)
    if(MUSICXML_FOUND)
        target_include_directories(libCsoundAC PRIVATE ${MUSICXML_INCLUDE_DIRS})
    endif()

    set_target_properties(libCsoundAC PROPERTIES
        SOVERSION "${APIVERSION}"
        OUTPUT_NAME CsoundAC
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
    )

    if(APPLE)
        set_target_properties(libCsoundAC PROPERTIES
            BUILD_RPATH "$ENV{HOME}/Library/Frameworks;/Library/Frameworks;/opt/homebrew/lib;/usr/local/lib"
            INSTALL_RPATH "@loader_path;$ENV{HOME}/Library/Frameworks;/Library/Frameworks;/opt/homebrew/lib;/usr/local/lib"
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
    elseif(UNIX)
        set_target_properties(libCsoundAC PROPERTIES
            BUILD_RPATH "$ORIGIN"
            INSTALL_RPATH "$ORIGIN"
        )
    endif()

    target_link_libraries(libCsoundAC PRIVATE ${MUSICXML2_LINK_LIBRARIES} ${CSOUND_LIBRARIES} ${LIBSNDFILE_LIBRARY})

    # Standalone test program (from: c++ ChordSpaceTest.cpp ... -o ChordSpaceTest)
    # User requested target name: ChordSpaceTests; adjust OUTPUT_NAME if desired.
    add_executable(ChordSpaceTests ChordSpaceTests.cpp)

    target_compile_definitions(ChordSpaceTests
    PRIVATE
        USE_DOUBLE
        $<$<STREQUAL:$<TARGET_PROPERTY:libCsoundAC,TYPE>,STATIC_LIBRARY>:CSOUNDAC_STATIC>
        CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
        CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
    )

    # Always prefer headers from this source tree over installed headers.
    target_include_directories(ChordSpaceTests
        BEFORE PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/CsoundAC
    )
    target_include_directories(ChordSpaceTests SYSTEM PRIVATE ${CSOUND_INCLUDE_DIRS})
    target_include_directories(ChordSpaceTests PRIVATE ${EIGEN3_INCLUDE_DIRS})
    target_include_directories(ChordSpaceTests PRIVATE ${Boost_INCLUDE_DIR})
    if(MUSICXML_FOUND)
        target_include_directories(ChordSpaceTests PRIVATE ${MUSICXML_INCLUDE_DIRS})
    endif()

    target_compile_options(ChordSpaceTests PRIVATE -Wno-write-strings)

    target_link_libraries(ChordSpaceTests PRIVATE
        libCsoundAC
        ${MUSICXML2_LINK_LIBRARIES}
        ${CSOUND_LIBRARIES}
        ${LIBSNDFILE_LIBRARY}
    )

    if(APPLE)
        set_target_properties(ChordSpaceTests PROPERTIES
            MACOSX_RPATH ON

            # Build tree layout:
            #   build-macos/CsoundAC/ChordSpaceTests
            #   build-macos/libCsoundAC.7.1.dylib
            BUILD_RPATH "@executable_path/.."

            # Install layout:
            #   <prefix>/bin/ChordSpaceTests
            #   <prefix>/lib/libCsoundAC.7.1.dylib
            INSTALL_RPATH "@executable_path/../lib"
        )
    endif()

    if(WIN32) # for MSYS2/mingw64
        # mingw64 needs this to create a valid .pyd.
        if(NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_compile_definitions(libCsoundAC PRIVATE -DMS_WIN64)
        endif()
        target_compile_definitions(libCsoundAC PRIVATE ${_csound_ac_windows_cimg_definitions})
        target_link_libraries(libCsoundAC PRIVATE ${JPEG_LIBRARY} ${PNG_LIBRARY} ${Z_LIBRARY})
    endif()

    if(HAVE_UNISTD_H)
        set_target_properties(libCsoundAC PROPERTIES
            COMPILE_FLAGS "-DHAVE_UNISTD_H"
        )
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIRECTORY}"
        DESTINATION "include/csound"
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(TARGETS libCsoundAC
        RUNTIME DESTINATION "${LIBRARY_INSTALL_DIR}"
        LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
        ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}"
    )

    if(MUSICXML2_RUNTIME_TARGET)
        install(TARGETS ${MUSICXML2_RUNTIME_TARGET}
            RUNTIME DESTINATION "${LIBRARY_INSTALL_DIR}"
            LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
        )
    elseif(MUSICXML2_RUNTIME_FILE)
        install(FILES "${MUSICXML2_RUNTIME_FILE}"
            DESTINATION "${LIBRARY_INSTALL_DIR}"
        )
    endif()

    if(SWIG_FOUND)
        message(STATUS "SWIG_USE_FILE: " ${SWIG_USE_FILE})
        include(${SWIG_USE_FILE})
        set(CMAKE_SWIG_OUTDIR ${BUILD_LIB_DIR})

        # SWIG also needs the local headers before any installed headers.
        include_directories(BEFORE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}/CsoundAC
        )

        list(APPEND CMAKE_SWIG_FLAGS
            "-includeall"
            "-verbose"
            "-I${CMAKE_CURRENT_SOURCE_DIR}"
            "-I${CMAKE_SOURCE_DIR}"
            "-I${CMAKE_SOURCE_DIR}/CsoundAC"
        )
    endif()

    if(BUILD_CSOUND_AC_PYTHON_INTERFACE)
        message(STATUS "Building CsoundAC Python interface.")
        message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
        message(STATUS "Python3_SITEARCH: ${Python3_SITEARCH}")
        message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
        include_directories(${Python3_INCLUDE_DIRS})

        # This variable must be set before swig_add_library.
        set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE)

        # Had: SWIG_FLAGS "-threads"
        set_source_files_properties(CsoundAC.i
            PROPERTIES CPLUSPLUS ON
            SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
            SWIG_MODULE_NAME CsoundAC
        )
        swig_add_library(CsoundAC
            LANGUAGE python
            SOURCES CsoundAC.i
        )
        target_include_directories(${SWIG_MODULE_CsoundAC_REAL_NAME}
            BEFORE PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_SOURCE_DIR}/CsoundAC
        )

        swig_link_libraries(CsoundAC libCsoundAC ${MUSICXML2_LINK_LIBRARIES} ${CSOUND_LIBRARIES} ${LIBSNDFILE_LIBRARY} ${Python3_LIBRARIES})

        target_include_directories(${SWIG_MODULE_CsoundAC_REAL_NAME}
            PRIVATE
                ${EIGEN3_INCLUDE_DIRS}
                ${Boost_INCLUDE_DIR}
                ${CSOUND_SOURCE_HOME}/include
        )
        target_include_directories(${SWIG_MODULE_CsoundAC_REAL_NAME} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/stb ${CMAKE_SOURCE_DIR}/dependencies/CImg ${LIBSNDFILE_INCLUDE_DIR})
        set_property(TARGET ${SWIG_MODULE_CsoundAC_REAL_NAME}
            PROPERTY SWIG_INCLUDE_DIRECTORIES
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_SOURCE_DIR}
                ${CMAKE_SOURCE_DIR}/CsoundAC
                ${EIGEN3_INCLUDE_DIRS}
                ${Boost_INCLUDE_DIR}
                ${CSOUND_SOURCE_HOME}/include
        )
        target_compile_definitions(${SWIG_MODULE_CsoundAC_REAL_NAME}
            PRIVATE
                USE_DOUBLE
                $<$<STREQUAL:$<TARGET_PROPERTY:libCsoundAC,TYPE>,STATIC_LIBRARY>:CSOUNDAC_STATIC>
                CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
                CSOUND_VERSION_MINOR=${CSOUND_VERSION_MINOR}
        )

        if(WIN32)
            target_compile_definitions(${SWIG_MODULE_CsoundAC_REAL_NAME}
                PRIVATE
                    ${_csound_ac_windows_cimg_definitions}
            )
        endif()

        if(APPLE)
            set_target_properties(${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTIES
                LINK_FLAGS "-undefined dynamic_lookup"
                BUILD_WITH_INSTALL_RPATH FALSE
                BUILD_RPATH "$ENV{HOME}/Library/Frameworks;/Library/Frameworks;/opt/homebrew/lib;/usr/local/lib;${BUILD_LIB_DIR}"
                INSTALL_RPATH "@loader_path/../..;@loader_path;$ENV{HOME}/Library/Frameworks;/Library/Frameworks;/opt/homebrew/lib;/usr/local/lib"
                INSTALL_RPATH_USE_LINK_PATH TRUE
            )
        elseif(UNIX)
            set_target_properties(${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTIES
                BUILD_RPATH "${BUILD_LIB_DIR}"
                INSTALL_RPATH "$ORIGIN/../..;$ORIGIN"
            )
        endif()

        target_include_directories(libCsoundAC SYSTEM PRIVATE ${CSOUND_INCLUDE_PATH})

        set_target_properties(${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
        )

        message(STATUS "SWIG_MODULE_CsoundAC_REAL_NAME: ${SWIG_MODULE_CsoundAC_REAL_NAME}")

        # For make install.
        #
        # When enabled, preserve the historical behavior: install the SWIG module
        # and Python files into the active interpreter's site-packages directory.
        #
        # On Linux and Windows, do not install into Python3_SITEARCH here. On CI
        # runners that expands to a real system interpreter directory such as
        # /usr/local/lib/python3.12/dist-packages, which is not writable and is
        # also not appropriate for release staging. The relative install rules
        # below place the Python interface inside the staged distribution tree.

        if(CSOUND_AC_INSTALL_PYTHON_TO_SITEARCH)
            install(TARGETS ${SWIG_MODULE_CsoundAC_REAL_NAME}
                LIBRARY DESTINATION "${Python3_SITEARCH}"
            )

            install(FILES "${BUILD_DIR}/CsoundAC.py"
                DESTINATION "${Python3_SITEARCH}"
            )

            install(FILES "${CMAKE_SOURCE_DIR}/silencio/python/GeneralizedContextualGroup.py"
                DESTINATION "${Python3_SITEARCH}"
            )

            install(FILES "${CMAKE_SOURCE_DIR}/reaper/ac_reaper.py"
                DESTINATION "${Python3_SITEARCH}"
            )
        endif()

        # For CPack/release ZIP staging on all platforms. These paths are
        # relative to CMAKE_INSTALL_PREFIX or to the --prefix supplied by
        # stage_dist, so they work in CI without writing to system locations.

        install(TARGETS "${SWIG_MODULE_CsoundAC_REAL_NAME}"
            RUNTIME DESTINATION "lib/python/site-packages"
            LIBRARY DESTINATION "lib/python/site-packages"
            ARCHIVE DESTINATION "lib/python/site-packages"
        )

        install(FILES "${BUILD_DIR}/CsoundAC.py"
            DESTINATION "lib/python/site-packages"
        )

        install(FILES "${CMAKE_SOURCE_DIR}/silencio/python/GeneralizedContextualGroup.py"
            DESTINATION "lib/python/site-packages"
        )

        install(FILES "${CMAKE_SOURCE_DIR}/reaper/ac_reaper.py"
            DESTINATION "lib/python/site-packages"
        )

    endif()

endif()
