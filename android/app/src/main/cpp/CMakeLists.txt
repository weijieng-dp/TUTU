# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.
# Sets the minimum CMake version required for this project.

cmake_minimum_required(VERSION 3.22.1)
# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("jnicpp")
# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking   # <--- THIS is required
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
        freetype
        GIT_REPOSITORY https://github.com/freetype/freetype.git
        GIT_TAG VER-2-13-3   # <--- THIS is required
)
FetchContent_MakeAvailable(freetype)
set(SRC_DIR "../../../../../src/GetMeOutOfHell")
set(Scripts_DIR "../../../../../src/Scripts")



add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/CoreLib CoreLib_build)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/Scripts Scripts_build)

include_directories(
        ${freetype_SOURCE_DIR}/include
)
target_link_libraries(CoreLib PRIVATE
        glm::glm
        freetype)
# Get source files from my_gl_app directory
file(GLOB MY_GL_APP_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/*.cpp"
        "${SRC_DIR}/*.h"
        "${SRC_DIR}/*.hpp"
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_android.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${imgui_SOURCE_DIR}/imgui.h
        ${imgui_SOURCE_DIR}/imgui_draw.h
        ${imgui_SOURCE_DIR}/imgui_tables.h
        ${imgui_SOURCE_DIR}/imgui_widgets.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_android.h
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        "${Scripts_DIR}/*.cpp"

        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h
        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
# Include directories
include_directories(${SRC_DIR})
include_directories(${CMAKE_CURRENT_LIST_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imgui_SOURCE_DIR}/misc/cpp
        ${CMAKE_CURRENT_LIST_DIR}/lib/fmod/inc
)
link_directories(${CMAKE_CURRENT_LIST_DIR}/lib/fmod/lib/${ANDROID_ABI})


include_directories("../../../../../src")
include_directories("../../../../../src/CoreLib")
include_directories("../../../../../src/Scripts")
add_library(jnicpp SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        NativeTemplate.cpp
        ${MY_GL_APP_SRC})

# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(jnicpp PRIVATE
        # List libraries link to the target library
        android
        log
        GLESv3
        EGL
        glm::glm
        CoreLib
        freetype
        #fmod libs
        ${CMAKE_CURRENT_LIST_DIR}/lib/fmod/lib/${ANDROID_ABI}/libfmod.so
        ${CMAKE_CURRENT_LIST_DIR}/lib/fmod/lib/${ANDROID_ABI}/libfmodL.so)