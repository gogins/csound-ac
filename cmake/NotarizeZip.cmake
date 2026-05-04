if(NOT APPLE)
    message(STATUS "NotarizeZip.cmake: non-Apple host; nothing to notarize.")
    return()
endif()

if(NOT CSOUND_AC_ENABLE_NOTARIZATION)
    message(STATUS "NotarizeZip.cmake: CSOUND_AC_ENABLE_NOTARIZATION is OFF; skipping notarization.")
    return()
endif()

if(NOT EXISTS "${CSOUND_AC_RELEASE_ZIP}")
    message(FATAL_ERROR "Release ZIP does not exist: ${CSOUND_AC_RELEASE_ZIP}")
endif()

set(_notary_args submit "${CSOUND_AC_RELEASE_ZIP}" --wait)

if(APPLE_NOTARYTOOL_PROFILE)
    list(APPEND _notary_args --keychain-profile "${APPLE_NOTARYTOOL_PROFILE}")
elseif(APPLE_NOTARY_KEY AND APPLE_NOTARY_KEY_ID AND APPLE_NOTARY_ISSUER_ID)
    list(APPEND _notary_args
        --key "${APPLE_NOTARY_KEY}"
        --key-id "${APPLE_NOTARY_KEY_ID}"
        --issuer "${APPLE_NOTARY_ISSUER_ID}"
    )
else()
    message(FATAL_ERROR "Notarization requested but no notarytool credentials were supplied. Set APPLE_NOTARYTOOL_PROFILE, or APPLE_NOTARY_KEY + APPLE_NOTARY_KEY_ID + APPLE_NOTARY_ISSUER_ID.")
endif()

message(STATUS "Submitting ZIP for notarization: ${CSOUND_AC_RELEASE_ZIP}")
execute_process(
    COMMAND /usr/bin/xcrun notarytool ${_notary_args}
    RESULT_VARIABLE _notary_result
)

if(NOT _notary_result EQUAL 0)
    message(FATAL_ERROR "notarytool submit failed for ${CSOUND_AC_RELEASE_ZIP}")
endif()

message(STATUS "Notarization accepted for ${CSOUND_AC_RELEASE_ZIP}")
message(STATUS "Note: ZIP files are notarized but are not stapled; stapling applies to app, pkg, and dmg containers.")
