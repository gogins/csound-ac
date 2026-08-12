# modelprompt — Csound 7 opcode plugin for generative-model interaction.
#
# Built as part of csound-ac when BUILD_MODELPROMPT is ON, or standalone:
#   cmake -S modelprompt -B modelprompt/build && cmake --build modelprompt/build

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.16)
    project(modelprompt LANGUAGES CXX)
    set(MODELPROMPT_STANDALONE ON)
else()
    set(MODELPROMPT_STANDALONE OFF)
endif()

find_package(CURL REQUIRED)

if(MODELPROMPT_STANDALONE)
    set(CSOUND_INCLUDE_DIR "" CACHE PATH "Csound 7 include directory")
    if(NOT CSOUND_INCLUDE_DIR)
        if(EXISTS "/Library/Frameworks/CsoundLib64.framework/Headers/csdl.h")
            set(CSOUND_INCLUDE_DIR "/Library/Frameworks/CsoundLib64.framework/Headers")
        elseif(EXISTS "$ENV{HOME}/Library/Frameworks/CsoundLib64.framework/Headers/csdl.h")
            set(CSOUND_INCLUDE_DIR "$ENV{HOME}/Library/Frameworks/CsoundLib64.framework/Headers")
        elseif(EXISTS "/usr/local/include/csound/csdl.h")
            set(CSOUND_INCLUDE_DIR "/usr/local/include/csound")
        elseif(EXISTS "/usr/include/csound/csdl.h")
            set(CSOUND_INCLUDE_DIR "/usr/include/csound")
        endif()
    endif()
    if(NOT CSOUND_INCLUDE_DIR)
        message(FATAL_ERROR "Csound headers not found; set CSOUND_INCLUDE_DIR")
    endif()
else()
    if(NOT CSOUND_INCLUDE_DIR AND CSOUND_INCLUDE_DIRS)
        list(GET CSOUND_INCLUDE_DIRS 0 CSOUND_INCLUDE_DIR)
    endif()
    if(NOT CSOUND_INCLUDE_DIR)
        message(FATAL_ERROR
            "modelprompt requires CSOUND_INCLUDE_DIR from the parent csound-ac build")
    endif()
endif()

message(STATUS "modelprompt CSOUND_INCLUDE_DIR: ${CSOUND_INCLUDE_DIR}")

add_library(modelprompt MODULE modelprompt.cpp)

target_include_directories(modelprompt PRIVATE ${CSOUND_INCLUDE_DIR})
# Parent csound-ac also adds ${CSOUND_INCLUDE_DIR}/csound; keep both for
# frameworks that put headers at the framework Headers root.
if(EXISTS "${CSOUND_INCLUDE_DIR}/csound/csdl.h")
    target_include_directories(modelprompt PRIVATE "${CSOUND_INCLUDE_DIR}/csound")
endif()

target_compile_features(modelprompt PRIVATE cxx_std_20)
target_compile_definitions(modelprompt PRIVATE BUILD_PLUGINS=1)
target_link_libraries(modelprompt PRIVATE CURL::libcurl)

if(APPLE)
    # Resolve Csound host symbols at load time.
    target_link_options(modelprompt PRIVATE
        "LINKER:-undefined,dynamic_lookup"
    )
    set_target_properties(modelprompt PROPERTIES
        BUNDLE FALSE
        PREFIX ""
        SUFFIX ".dylib"
    )
elseif(WIN32)
    # Windows plugins must link the import library for host symbols.
    if(CSOUND_LIBRARIES)
        target_link_libraries(modelprompt PRIVATE ${CSOUND_LIBRARIES})
    endif()
    # CommandLineToArgvW for per-.csd cache path discovery.
    target_link_libraries(modelprompt PRIVATE shell32)
    set_target_properties(modelprompt PROPERTIES
        PREFIX ""
    )
else()
    set_target_properties(modelprompt PROPERTIES
        PREFIX ""
    )
endif()

# Standard Csound 7 64-bit plugin location under the install prefix.
if(NOT MODELPROMPT_PLUGIN_INSTALL_DIR)
    set(MODELPROMPT_PLUGIN_INSTALL_DIR "lib/csound/plugins64-7.0")
endif()

install(TARGETS modelprompt
    LIBRARY DESTINATION "${MODELPROMPT_PLUGIN_INSTALL_DIR}"
    RUNTIME DESTINATION "${MODELPROMPT_PLUGIN_INSTALL_DIR}"
    COMPONENT csound-ac
)

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
    DESTINATION "share/doc/csound-ac/modelprompt"
    COMPONENT csound-ac
)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/examples/"
    DESTINATION "share/doc/csound-ac/modelprompt/examples"
    COMPONENT csound-ac
    FILES_MATCHING PATTERN "*.csd"
)
