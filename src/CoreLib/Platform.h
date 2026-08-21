/**___________________________________________________________________________/
@file		   platform.h
@author        parminder.singh@digipen.edu
@date          16/09/2025

This file defines the type of platform we are using and also provide the cross platform
LOGI function for us to debug information

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/


#ifndef PLATFORM_H
#define PLATFORM_H
//#undef _WIN32
//#define __ANDROID__
// Platform detection
#ifdef _WIN32
#define PLATFORM_WINDOWS
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
#endif
// Platform-specific includes
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include "CEO.h"
#include "Logger.h"
// Windows logging macros
#include <cstdio>
#ifdef EditorFlag
#define LOGI(...) CEO::Instance().GetManager<Logger>()->AddLog(CEO::Instance().GetManager<Logger>()->FormatString(__VA_ARGS__),Logger::LogLevel::Info);
#define LOGE(...) CEO::Instance().GetManager<Logger>()->AddLog(CEO::Instance().GetManager<Logger>()->FormatString(__VA_ARGS__),Logger::LogLevel::Error);
#define LOGD(...) CEO::Instance().GetManager<Logger>()->AddLog(CEO::Instance().GetManager<Logger>()->FormatString(__VA_ARGS__),Logger::LogLevel::Warning);
#else
#define LOGI(...) printf("");
#define LOGE(...)  printf("");
#define LOGD(...)  printf("");
#endif
#elif defined(PLATFORM_ANDROID)
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/log.h>
#include <cstring>
// Android logging macros (LOG_TAG should be defined in each source file)
#ifndef LOGI
#define LOG_TAG "CPP PROJ"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#endif
#endif

#ifdef EditorFlag
#define DEBUGBREAK __debugbreak();
#else
#define DEBUGBREAK
#endif

#endif // PLATFORM_H
