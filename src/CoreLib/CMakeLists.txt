cmake_minimum_required(VERSION 3.16)
include(FetchContent)

# === Collect CoreLib sources ===
file(GLOB_RECURSE CORELIB_sources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/*.h
)

# === stb (header-only, no target) ===
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
	FetchContent_MakeAvailable(stb)
endif()

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG        v1.1.0
)

# Download only, do NOT auto-configure
FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()


# === Build CoreLib ===
add_library(CoreLib STATIC ${CORELIB_sources})

# CoreLib needs its own headers + stb
target_include_directories(CoreLib
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}    # CoreLib headers
        ${stb_SOURCE_DIR}            # stb headers
 	${rapidjson_SOURCE_DIR}/include
)

# Precompiled Headers
target_precompile_headers(CoreLib
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/pch.h
)

if(ANDROID)
    target_include_directories(CoreLib PUBLIC
        ${CMAKE_SOURCE_DIR}/lib/fmod/inc
    )
endif()

# Optional: put archive in lib/CoreLib only for desktop (not Android/NDK builds)
if (NOT ANDROID)
    set_target_properties(CoreLib PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/CoreLib"
    )
endif()
