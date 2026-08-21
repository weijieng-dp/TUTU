cmake_minimum_required(VERSION 3.15)

# ==========================================
# Setup vcpkg as an embedded dependency manager
# ==========================================
set(VCPKG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/lib/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

# If vcpkg is present, force its toolchain
if(EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file" FORCE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(WARNING "vcpkg not found in ${VCPKG_ROOT}. Did you clone with --recurse-submodules?")
endif()

# ==========================================
# Project definition
# ==========================================
project(GAM200-ENGINE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CONFIGURATION_TYPES
    "Debug;Release;RelWithDebInfo;MinSizeRel;Package"
   CACHE STRING "" FORCE)

add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)


set(CMAKE_MAP_IMPORTED_CONFIG_PACKAGE "Release")

add_compile_definitions(
    $<$<CONFIG:Debug>:EditorFlag>
   $<$<CONFIG:Release>:EditorFlag>
)

# Global defines
add_definitions(-DNOMINMAX)
add_definitions(-DGLFW_DLL)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(NOT ANDROID)
#add_definitions(-DEditorFlag)
endif()

if (MSVC)
    add_compile_options(/W4)
endif()

# ==========================================
# Auto-bootstrap vcpkg
# ==========================================
if(EXISTS "${VCPKG_ROOT}" AND NOT EXISTS "${VCPKG_ROOT}/vcpkg")
    message(STATUS "Bootstrapping vcpkg...")
    if(WIN32)
        execute_process(COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.bat" "-disableMetrics")
    else()
        execute_process(COMMAND "${VCPKG_ROOT}/bootstrap-vcpkg.sh" "-disableMetrics")
    endif()
endif()

# ==========================================
# Subprojects
# ==========================================





#find_package(rttr CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(rapidjson CONFIG REQUIRED)
find_path(REFL_CPP_INCLUDE_DIRS "refl.hpp" CONFIG REQUIRED)


add_subdirectory(src/Scripts)
add_subdirectory(src/CoreLib)
add_subdirectory(src/GetMeOutOfHell)


if (MSVC)
    target_compile_options(Scripts PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:/bigobj>
    )
endif()


target_link_options(GetMeOutOfHell PRIVATE
    "$<IF:$<OR:$<CONFIG:Package>,$<CONFIG:MinSizeRel>>,/SUBSYSTEM:WINDOWS,>"
)


add_dependencies(GetMeOutOfHell CoreLib) 
#add_dependencies(Scripts CoreLib)

target_link_directories(CoreLib PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/lib/x64
)

target_link_directories(Scripts PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/lib/x64
)
target_link_libraries(CoreLib
    PRIVATE
	GLEW::GLEW
	glm::glm
	Freetype::Freetype
	debug fmodL_vc
	optimized fmod_vc
)
target_link_libraries( GetMeOutOfHell
    PRIVATE
	GLEW::GLEW
	glfw
	imgui::imgui
	glm::glm
	Freetype::Freetype
	debug fmodL_vc
	optimized fmod_vc
	implot::implot
    imguizmo::imguizmo
)

target_link_libraries(Scripts
    PRIVATE
	GLEW::GLEW
	glm::glm
	Freetype::Freetype
	debug fmodL_vc
	optimized fmod_vc
#CoreLib
)
target_include_directories(CoreLib PRIVATE
    ${REFL_CPP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/inc
${RAPIDJSON_INCLUDE_DIRS}
)

target_include_directories(Scripts PRIVATE
    ${REFL_CPP_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/inc
  $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/CoreLib/Debug>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/CoreLib/Release>
${RAPIDJSON_INCLUDE_DIRS}
)

target_include_directories(GetMeOutOfHell
    PRIVATE
  ${REFL_CPP_INCLUDE_DIRS}
  ${FREETYPE_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmod/inc
${RAPIDJSON_INCLUDE_DIRS}
)


# After defining all executables/libraries
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GetMeOutOfHell)

# ==========================================
# Organize IDE filters automatically
# ==========================================
function(set_source_groups target_name)
    get_target_property(target_sources ${target_name} SOURCES)
    foreach(source ${target_sources})
        get_filename_component(source_path "${source}" PATH)
        file(RELATIVE_PATH rel_source_path ${CMAKE_SOURCE_DIR} ${source_path})
        if(rel_source_path)
            string(REPLACE "\\" "/" source_group_path "${rel_source_path}")
            source_group("${source_group_path}" FILES "${source}")
        endif()
    endforeach()
endfunction()


set_source_groups(GetMeOutOfHell)