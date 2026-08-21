cmake_minimum_required(VERSION 3.5)
# === Collect CoreLib sources ===
file(GLOB_RECURSE CORELIB_sources CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.cpp
    ${CMAKE_CURRENT_LIST_DIR}/*.h
)

file(GLOB_RECURSE Engine_sources CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/CoreLib/*.cpp
${CMAKE_CURRENT_LIST_DIR}/*.h
)


# === Build CoreLib ===
if(ANDROID)
add_library(Scripts STATIC ${CORELIB_sources})
else()
add_library(Scripts SHARED 
${CORELIB_sources}
${Engine_sources}
)

if(MSVC)
    #target_compile_options(Scripts PRIVATE /Z7)
    #set_target_properties(Scripts PROPERTIES LINK_FLAGS "/DEBUG:NONE")
endif()


endif()

add_definitions(-DNativeScripting)