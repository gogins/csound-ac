cmake_minimum_required(VERSION 3.16)
project(modelprompt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

# Prefer installed / built Csound 7 headers (must include generated float-version.h).
set(CSOUND_INCLUDE_DIR "" CACHE PATH "Csound 7 include directory")
if(NOT CSOUND_INCLUDE_DIR)
  if(EXISTS "/Library/Frameworks/CsoundLib64.framework/Headers/csdl.h")
    set(CSOUND_INCLUDE_DIR "/Library/Frameworks/CsoundLib64.framework/Headers")
  elseif(EXISTS "/Users/michaelgogins/csound/build/CsoundLib64.framework/Versions/7.0/Headers/csdl.h")
    set(CSOUND_INCLUDE_DIR "/Users/michaelgogins/csound/build/CsoundLib64.framework/Versions/7.0/Headers")
  elseif(EXISTS "/usr/local/include/csound/csdl.h")
    set(CSOUND_INCLUDE_DIR "/usr/local/include/csound")
  endif()
endif()

if(NOT CSOUND_INCLUDE_DIR)
  message(FATAL_ERROR "Csound headers not found; set CSOUND_INCLUDE_DIR")
endif()

add_library(modelprompt MODULE modelprompt.cpp)
target_include_directories(modelprompt PRIVATE ${CSOUND_INCLUDE_DIR})
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
else()
  set_target_properties(modelprompt PROPERTIES
    PREFIX ""
  )
endif()

install(TARGETS modelprompt DESTINATION lib/csound/plugins64-7.0)
