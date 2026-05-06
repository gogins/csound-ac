if(NOT APPLE)
    message(STATUS "NotarizeZip.cmake: non-Apple host; nothing to notarize.")
    return()
endif()

if(NOT CSOUND_AC_ENABLE_NOTARIZATION)
    message(STATUS "NotarizeZip.cmake: CSOUND_AC_ENABLE_NOTARIZATION is OFF; skipping notarization.")
    return()
endif()

if(NOT DEFINED CSOUND_AC_ARCHIVE OR CSOUND_AC_ARCHIVE STREQUAL "")
    message(FATAL_ERROR "CSOUND_AC_ARCHIVE was not supplied.")
endif()

if(NOT EXISTS "${CSOUND_AC_ARCHIVE}")
    message(FATAL_ERROR "Release ZIP does not exist: ${CSOUND_AC_ARCHIVE}")
endif()

set(_notary_args submit "${CSOUND_AC_ARCHIVE}" --wait)

if(APPLE_NOTARYTOOL_PROFILE)
    list(APPEND _notary_args
        --keychain-profile "${APPLE_NOTARYTOOL_PROFILE}"
    )
elseif(APPLE_NOTARY_KEY AND APPLE_NOTARY_KEY_ID AND APPLE_NOTARY_ISSUER_ID)
    list(APPEND _notary_args
        --key "${APPLE_NOTARY_KEY}"
        --key-id "${APPLE_NOTARY_KEY_ID}"
        --issuer "${APPLE_NOTARY_ISSUER_ID}"
    )
else()
    message(FATAL_ERROR
        "Notarization requested but no notarytool credentials were supplied. "
        "Set APPLE_NOTARYTOOL_PROFILE, or "
        "APPLE_NOTARY_KEY + APPLE_NOTARY_KEY_ID + APPLE_NOTARY_ISSUER_ID."
    )
endif()

message(STATUS "Submitting ZIP for notarization: ${CSOUND_AC_ARCHIVE}")

execute_process(
    COMMAND /usr/bin/xcrun notarytool ${_notary_args}
    RESULT_VARIABLE _notary_result
)

if(NOT _notary_result EQUAL 0)
    message(FATAL_ERROR "notarytool submit failed for ${CSOUND_AC_ARCHIVE}")
endif()

message(STATUS "Notarization accepted for ${CSOUND_AC_ARCHIVE}")
message(STATUS "Note: ZIP files are notarized but are not stapled; stapling applies to app, pkg, and dmg containers.")
