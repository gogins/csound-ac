# Release/staging/signing/notarization targets for csound-ac.
#
# Include this from the top-level CMakeLists.txt after install() rules have been
# declared. The top-level CMakeLists.txt owns CSOUND_AC_ARCHIVE so local builds
# and GitHub Actions use the same archive path.

include_guard(GLOBAL)

set(CSOUND_AC_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}" CACHE STRING "csound-ac release version")
if(NOT CSOUND_AC_PACKAGE_VERSION)
    set(CSOUND_AC_PACKAGE_VERSION "8.0.0")
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" _csound_ac_system_name_lower)
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CSOUND_AC_PLATFORM_NAME "macos" CACHE STRING "Platform name used in release artifact names")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CSOUND_AC_PLATFORM_NAME "windows" CACHE STRING "Platform name used in release artifact names")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CSOUND_AC_PLATFORM_NAME "linux" CACHE STRING "Platform name used in release artifact names")
else()
    set(CSOUND_AC_PLATFORM_NAME "${_csound_ac_system_name_lower}" CACHE STRING "Platform name used in release artifact names")
endif()

set(CSOUND_AC_DIST_DIR "${CMAKE_SOURCE_DIR}/dist" CACHE PATH "Directory used for staged and packaged release artifacts")
set(CSOUND_AC_DIST_ROOT "${CSOUND_AC_DIST_DIR}/csound-ac" CACHE PATH "Staged install root inside dist")

if(NOT DEFINED CSOUND_AC_ARCHIVE OR CSOUND_AC_ARCHIVE STREQUAL "")
    message(FATAL_ERROR
        "CSOUND_AC_ARCHIVE must be defined by the top-level CMakeLists.txt before "
        "including cmake/CsoundACRelease.cmake. Expected example: "
        "set(CSOUND_AC_ARCHIVE \"\${CMAKE_BINARY_DIR}/\${CPACK_PACKAGE_NAME}-\${CPACK_PACKAGE_VERSION}-\${CMAKE_SYSTEM_NAME}.zip\" CACHE FILEPATH \"...\")"
    )
endif()

message(STATUS "CSOUND_AC_DIST_DIR:  ${CSOUND_AC_DIST_DIR}")
message(STATUS "CSOUND_AC_DIST_ROOT: ${CSOUND_AC_DIST_ROOT}")
message(STATUS "CSOUND_AC_ARCHIVE:   ${CSOUND_AC_ARCHIVE}")

option(CSOUND_AC_ENABLE_CODESIGN "Codesign staged macOS binaries before packaging" OFF)
option(CSOUND_AC_ENABLE_NOTARIZATION "Submit the macOS release ZIP to Apple notarization" OFF)

set(APPLE_CODESIGN_IDENTITY "$ENV{APPLE_CODESIGN_IDENTITY}" CACHE STRING "Developer ID Application signing identity")
set(APPLE_NOTARYTOOL_PROFILE "$ENV{APPLE_NOTARYTOOL_PROFILE}" CACHE STRING "notarytool keychain profile name")
set(APPLE_NOTARY_KEY "$ENV{APPLE_NOTARY_KEY}" CACHE FILEPATH "Path to App Store Connect API key .p8 file")
set(APPLE_NOTARY_KEY_ID "$ENV{APPLE_NOTARY_KEY_ID}" CACHE STRING "App Store Connect API key ID")
set(APPLE_NOTARY_ISSUER_ID "$ENV{APPLE_NOTARY_ISSUER_ID}" CACHE STRING "App Store Connect API issuer ID")

add_custom_target(stage_dist
    COMMAND "${CMAKE_COMMAND}" -E rm -rf "${CSOUND_AC_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${CSOUND_AC_DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --prefix "${CSOUND_AC_DIST_ROOT}"
    COMMENT "Staging csound-ac install tree in ${CSOUND_AC_DIST_ROOT}"
    VERBATIM
)

if(APPLE)
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND "${CMAKE_COMMAND}"
            "-DCSOUND_AC_DIST_ROOT=${CSOUND_AC_DIST_ROOT}"
            "-DCSOUND_AC_ENABLE_CODESIGN:BOOL=${CSOUND_AC_ENABLE_CODESIGN}"
            "-DAPPLE_CODESIGN_IDENTITY=${APPLE_CODESIGN_IDENTITY}"
            -P "${CMAKE_SOURCE_DIR}/cmake/SignDist.cmake"
        COMMENT "Signing staged macOS binaries in ${CSOUND_AC_DIST_ROOT}"
        VERBATIM
    )
else()
    add_custom_target(sign_dist
        DEPENDS stage_dist
        COMMAND "${CMAKE_COMMAND}" -E echo "Codesigning skipped on ${CMAKE_SYSTEM_NAME}."
        COMMENT "Codesigning skipped on non-macOS platforms"
        VERBATIM
    )
endif()

add_custom_target(package_dist
    DEPENDS sign_dist
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${CSOUND_AC_ARCHIVE}"
    COMMAND "${CMAKE_COMMAND}" -E tar cfv "${CSOUND_AC_ARCHIVE}" --format=zip "csound-ac"
    WORKING_DIRECTORY "${CSOUND_AC_DIST_DIR}"
    COMMENT "Creating ${CSOUND_AC_ARCHIVE}"
    VERBATIM
)

if(APPLE)
    add_custom_target(notarize_dist
        DEPENDS package_dist
        COMMAND "${CMAKE_COMMAND}"
            "-DCSOUND_AC_ARCHIVE=${CSOUND_AC_ARCHIVE}"
            "-DCSOUND_AC_ENABLE_NOTARIZATION:BOOL=${CSOUND_AC_ENABLE_NOTARIZATION}"
            "-DAPPLE_NOTARYTOOL_PROFILE=${APPLE_NOTARYTOOL_PROFILE}"
            "-DAPPLE_NOTARY_KEY=${APPLE_NOTARY_KEY}"
            "-DAPPLE_NOTARY_KEY_ID=${APPLE_NOTARY_KEY_ID}"
            "-DAPPLE_NOTARY_ISSUER_ID=${APPLE_NOTARY_ISSUER_ID}"
            -P "${CMAKE_SOURCE_DIR}/cmake/NotarizeZip.cmake"
        COMMENT "Notarizing ${CSOUND_AC_ARCHIVE}"
        VERBATIM
    )
else()
    add_custom_target(notarize_dist
        DEPENDS package_dist
        COMMAND "${CMAKE_COMMAND}" -E echo "Notarization skipped on ${CMAKE_SYSTEM_NAME}."
        COMMENT "Notarization skipped on non-macOS platforms"
        VERBATIM
    )
endif()

add_custom_target(release_dist
    DEPENDS notarize_dist
    COMMENT "Built staged, signed, packaged release artifact: ${CSOUND_AC_ARCHIVE}"
)
