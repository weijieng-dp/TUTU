cmake_minimum_required(VERSION 3.5)

#find files with extension of .cpp and .h
file(GLOB_RECURSE Engine_source_files CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.[ch]pp
    ${CMAKE_CURRENT_LIST_DIR}/*.h
)


#adding project
add_executable(GetMeOutOfHell
    ${Engine_source_files}
    ${imgui_source_files}
)

#comment this part if you want to enable console window
if (MSVC)
    #target_link_options(GetMeOutOfHell PRIVATE 
     #   "/SUBSYSTEM:WINDOWS"
   #  )

endif()

set_target_properties(GetMeOutOfHell PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/src"
)

#include directories
target_include_directories(GetMeOutOfHell PRIVATE
    ${CMAKE_SOURCE_DIR}/src/CoreLib
)


target_link_directories(GetMeOutOfHell PRIVATE
    $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/lib/CoreLib/Debug>
    $<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/lib/CoreLib/Release>
    $<$<CONFIG:Package>:${CMAKE_SOURCE_DIR}/lib/CoreLib/Package>
    ${CMAKE_SOURCE_DIR}/lib/fmod/lib/x64
)

target_link_libraries(GetMeOutOfHell PRIVATE
   opengl32.lib
   CoreLib.lib
)
# Debug & Release DLLs


# Copy assets & shaders folder
add_custom_command(TARGET GetMeOutOfHell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        $<TARGET_FILE_DIR:GetMeOutOfHell>/Assets

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
	"${CMAKE_SOURCE_DIR}/build/src/GetMeOutOfHell/Assets"

   COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<$<CONFIG:Debug>:"${CMAKE_SOURCE_DIR}/lib/fmod/lib/x64/fmodL.dll">
        $<$<CONFIG:Release>:"${CMAKE_SOURCE_DIR}/lib/fmod/lib/x64/fmod.dll">
        $<$<CONFIG:Package>:"${CMAKE_SOURCE_DIR}/lib/fmod/lib/x64/fmod.dll">
        $<TARGET_FILE_DIR:GetMeOutOfHell>

COMMAND ${CMAKE_COMMAND} -E copy_if_different
	$<$<CONFIG:Debug>:"${CMAKE_SOURCE_DIR}/build/src/Scripts/Debug/Scripts.dll">
	$<$<CONFIG:Release>:"${CMAKE_SOURCE_DIR}/build/src/Scripts/Release/Scripts.dll">
	$<$<CONFIG:Package>:"${CMAKE_SOURCE_DIR}/build/src/Scripts/Package/Scripts.dll">
        $<TARGET_FILE_DIR:GetMeOutOfHell>


)

