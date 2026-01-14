# Root of Steamworks SDK
set(STEAMWORKS_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/steamworks-sdk"
        CACHE PATH "Steamworks SDK root")

set(STEAMWORKS_INCLUDE_DIR
        "${STEAMWORKS_ROOT}/public")

# ---- Platform libraries ----

if (WIN32)
    if ((CMAKE_SYSTEM_PROCESSOR MATCHES "ARM|ARM64|AARCH64"))
        message("Steamwork does not support windows on arm")
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(STEAMWORKS_LIB
                "${STEAMWORKS_ROOT}/redistributable_bin/win64/steam_api64.lib")
        set(STEAMWORKS_DLL
                "${STEAMWORKS_ROOT}/redistributable_bin/win64/steam_api64.dll")
    else ()
        message(FATAL_ERROR "32-bit Windows not supported by Steamworks anymore")
    endif ()

elseif (APPLE)
    set(STEAMWORKS_LIB
            "${STEAMWORKS_ROOT}/redistributable_bin/osx/libsteam_api.dylib")

elseif (UNIX)
    set(STEAMWORKS_LIB
            "${STEAMWORKS_ROOT}/redistributable_bin/linux64/libsteam_api.so")
endif ()

if (DEFINED STEAMWORKS_LIB)
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(
            Steamworks
            DEFAULT_MSG
            STEAMWORKS_LIB
            STEAMWORKS_INCLUDE_DIR
    )

    if (STEAMWORKS_FOUND)
        add_library(Steamworks SHARED IMPORTED)
        set_target_properties(Steamworks PROPERTIES
                IMPORTED_LOCATION "${STEAMWORKS_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${STEAMWORKS_INCLUDE_DIR}"
        )
    endif ()
endif ()