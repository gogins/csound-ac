if(NOT APPLE)
    message(STATUS "SignDist.cmake: non-Apple host; nothing to sign.")
    return()
endif()

if(NOT CSOUND_AC_ENABLE_CODESIGN)
    message(STATUS "SignDist.cmake: CSOUND_AC_ENABLE_CODESIGN is OFF; skipping codesign.")
    return()
endif()

if(NOT APPLE_CODESIGN_IDENTITY)
    message(FATAL_ERROR "CSOUND_AC_ENABLE_CODESIGN is ON but APPLE_CODESIGN_IDENTITY is empty.")
endif()

if(NOT EXISTS "${CSOUND_AC_DIST_ROOT}")
    message(FATAL_ERROR "Staged dist root does not exist: ${CSOUND_AC_DIST_ROOT}")
endif()

file(GLOB_RECURSE _candidates
    LIST_DIRECTORIES false
    "${CSOUND_AC_DIST_ROOT}/*"
)

set(_signed_count 0)
foreach(_file IN LISTS _candidates)
    get_filename_component(_ext "${_file}" EXT)
    set(_should_sign OFF)

    if(_ext MATCHES "\\.(dylib|so|pyd|bundle|plugin|vst3|component|app)$")
        set(_should_sign ON)
    endif()

    if(NOT _should_sign)
        execute_process(
            COMMAND /usr/bin/file -b "${_file}"
            OUTPUT_VARIABLE _file_type
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_file_type MATCHES "Mach-O")
            set(_should_sign ON)
        endif()
    endif()

    if(_should_sign)
        message(STATUS "codesign: ${_file}")
        execute_process(
            COMMAND /usr/bin/codesign
                --force
                --timestamp
                --options runtime
                --sign "${APPLE_CODESIGN_IDENTITY}"
                "${_file}"
            RESULT_VARIABLE _codesign_result
        )
        if(NOT _codesign_result EQUAL 0)
            message(FATAL_ERROR "codesign failed for ${_file}")
        endif()
        math(EXPR _signed_count "${_signed_count} + 1")
    endif()
endforeach()

message(STATUS "Signed ${_signed_count} file(s) in ${CSOUND_AC_DIST_ROOT}")
