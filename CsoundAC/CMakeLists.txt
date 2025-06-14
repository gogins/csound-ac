# Build CsoundAC
cmake_minimum_required(VERSION 3.28+)
include(CheckIncludeFiles)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)

add_definitions(-fPIC)

set(CMAKE_INCLUDE_CURRENT_DIR "On")

option(BUILD_CSOUND_AC "Build CsoundAC extension module for Csound with algorithmic composition (needs C++ interface)" ON)
option(BUILD_CSOUND_AC_PYTHON_INTERFACE "Build the Csound AC Python interface" ON)

find_package(SWIG)

# /opt/homebrew/Cellar/python@3.12/3.12.7_1/Frameworks/Python.framework/Versions/3.12/lib/libpython3.12.dylib -> ../Python)
find_package (Python3 3.12 EXACT COMPONENTS Interpreter Development)
message(STATUS "Python libraries:  ${PYTHONLIBS_VERSION_STRING}")

# Prefer the static MusicXML library as there is no system package for it.

set(${MUSICXML2_LIBRARY} "${CMAKE_SOURCE_DIR}/dependencies/build/lib/libmusicxml2.a")

list(APPEND CMAKE_SWIG_FLAGS
"-I${CMAKE_SOURCE_DIR}/dependencies/csound/interfaces")

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
        trace.cpp)
    message(STATUS "libcsoundac_SRCS: " ${libcsoundac_SRCS})

    if (MSVC)
        add_library(libCsoundAC STATIC ${libcsoundac_SRCS})
    else()
        add_library(libCsoundAC SHARED ${libcsoundac_SRCS})
    endif()
    
    target_include_directories(libCsoundAC SYSTEM PRIVATE ${Python3_INCLUDE_DIRS})
    target_include_directories(libCsoundAC SYSTEM PRIVATE ${CSOUND_INCLUDE_DIRS})
    target_include_directories(libCsoundAC PRIVATE ${EIGEN3_INCLUDE_DIRS})
    target_include_directories(libCsoundAC PRIVATE ${Boost_INCLUDE_DIR})
    target_include_directories(libCsoundAC PRIVATE ../dependencies/portsmf)
    target_include_directories(libCsoundAC PRIVATE ../dependencies/stb)
    target_include_directories(libCsoundAC PRIVATE ../dependencies/CImg)
    if (MUSICXML_FOUND)
        target_include_directories(libCsoundAC PRIVATE ${MUSICXML_INCLUDE_DIRS})
    endif()

    set_target_properties(libCsoundAC PROPERTIES
        SOVERSION "${APIVERSION}"
        OUTPUT_NAME CsoundAC
        RUNTIME_OUTPUT_DIRECTORY "${BUILD_BIN_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
        ARCHIVE_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}")

    target_link_libraries(libCsoundAC PRIVATE ${MUSICXML2_LIBRARY} ${CSOUND_LIBRARIES} ${LIBSNDFILE_LIBRARY})
 
    if(WIN32) # for MSYS2/mingw64
        #  mingw64 needs this to create a valid .pyd.
        if(NOT MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_compile_definitions(libCsoundAC PRIVATE -DMS_WIN64)
        endif()
        target_link_libraries(libCsoundAC PRIVATE ${JPEG_LIBRARY} ${PNG_LIBRARY} ${Z_LIBRARY})
    endif()

    if(HAVE_UNISTD_H)
        set_target_properties(libCsoundAC PROPERTIES
        COMPILE_FLAGS "-DHAVE_UNISTD_H")
    endif()

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIRECTORY}"
        DESTINATION "include/csound"
        FILES_MATCHING PATTERN "*.hpp")

    install(TARGETS libCsoundAC
        LIBRARY DESTINATION "${LIBRARY_INSTALL_DIR}"
        ARCHIVE DESTINATION "${LIBRARY_INSTALL_DIR}")

    if(SWIG_FOUND)
        message(STATUS "SWIG_USE_FILE: " ${SWIG_USE_FILE})
        include(${SWIG_USE_FILE})
        set(CMAKE_SWIG_OUTDIR ${BUILD_LIB_DIR})
        include_directories(${CMAKE_CURRENT_SOURCE_DIR})
        list(APPEND CMAKE_SWIG_FLAGS "-includeall" "-verbose")
    endif()

    if(BUILD_CSOUND_AC_PYTHON_INTERFACE)
        message(STATUS "Building CsoundAC Python interface.")
        message(STATUS "CMAKE_INSTALL_LIBDIR: ${CMAKE_INSTALL_LIBDIR}")
        message(STATUS "Python3_SITEARCH: ${Python3_SITEARCH}")
        message(STATUS "Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
        include_directories(${Python3_INCLUDE_DIRS})
        # Had: 	    SWIG_FLAGS "-threads"
        set_source_files_properties(CsoundAC.i
            PROPERTIES CPLUSPLUS ON
            SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON
            SWIG_MODULE_NAME CsoundAC)
        swig_add_library(CsoundAC 
            LANGUAGE python 
            SOURCES CsoundAC.i)
        swig_link_libraries(CsoundAC libCsoundAC ${MUSICXML2_LIBRARY} ${CSOUND_LIBRARIES} ${LIBSNDFILE_LIBRARY} ${Python3_LIBRARIES})
        target_include_directories(${SWIG_MODULE_CsoundAC_REAL_NAME} PRIVATE ${EIGEN3_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} ${CSOUND_SOURCE_HOME}/include)
        set(SWIG_USE_TARGET_INCLUDE_DIRECTORIES True)
        target_include_directories(${SWIG_MODULE_CsoundAC_REAL_NAME} SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/dependencies/stb ${CMAKE_SOURCE_DIR}/dependencies/CImg ${LIBSNDFILE_INCLUDE_DIR})
        set_property(TARGET ${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTY SWIG_INCLUDE_DIRECTORIES ${EIGEN3_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
 
        if(APPLE)
            set_target_properties(${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTIES
                LINK_FLAGS "-undefined dynamic_lookup")
        endif()

        target_include_directories(libCsoundAC SYSTEM PRIVATE . ${CSOUND_INCLUDE_PATH})
        
        set_target_properties(${SWIG_MODULE_CsoundAC_REAL_NAME} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${BUILD_LIB_DIR}")

        message(STATUS "SWIG_MODULE_CsoundAC_REAL_NAME: ${SWIG_MODULE_CsoundAC_REAL_NAME}")

        # For make install.
        
        install(TARGETS ${SWIG_MODULE_CsoundAC_REAL_NAME}
            LIBRARY DESTINATION "${Python3_SITEARCH}")
            
        install(FILES "${BUILD_DIR}/CsoundAC.py"
            DESTINATION "${Python3_SITEARCH}")
            
        install(FILES "${CMAKE_SOURCE_DIR}/silencio/python/GeneralizedContextualGroup.py"
            DESTINATION "${Python3_SITEARCH}")

        install(FILES "${CMAKE_SOURCE_DIR}/reaper/ac_reaper.py"
            DESTINATION "${Python3_SITEARCH}")

        # For CPack to ZIP.

        install(TARGETS "${SWIG_MODULE_CsoundAC_REAL_NAME}"
            DESTINATION "lib/python/site-packages")
            
        install(FILES "${BUILD_DIR}/CsoundAC.py"
            DESTINATION "lib/python/site-packages")
            
        install(FILES "${CMAKE_SOURCE_DIR}/silencio/python/GeneralizedContextualGroup.py"
            DESTINATION "lib/python/site-packages")

        install(FILES "${CMAKE_SOURCE_DIR}/reaper/ac_reaper.py"
            DESTINATION "lib/python/site-packages")
     
    endif()

endif()
