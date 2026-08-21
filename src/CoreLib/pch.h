/**___________________________________________________________________________/
@file       pch.h
@author     Tan Jun Jie (t.junjie) (100%)
@date		03/02/2026 (DD/MM/YYYY)
@brief		Simple pre-compile header.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once

// ----------------------------
// Standard Library
// ----------------------------
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <stack>
#include <memory>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <typeindex>
#include <typeinfo>
#include <bitset>
#include <utility>
#include <iterator>
#include <random>

// ----------------------------
// Math
// ----------------------------
#include <cmath>
#include <limits>
#include <cfloat>
#include <stdint.h>

// ----------------------------
// Containers / Utilities
// ----------------------------
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <chrono>

/* Engine Related Includes are:
====================================
AudioManager:           audio.h
Camera.h:               Platform.h, Registry.h, MathLib.h
EventsDispatcher.h:     Events.h
FontManager.h:          font.h
glslshader.h:           Platform.h, MathLib.h
HierarchyManager.h:     Registry.h
InputManager.h:         RenderUtils.h, glslshader.h, Platform.h
LayerManager.h:         Json.h
ParticleSystem.h:       MathLib.h
ResourceManager.h:      audio.h, texture.h, font.h, Animation.h, glslshader.h, Json.h, Registry.h
RenderUtils.h:          Platform.h, RenderAttributes.h, MathLib.h, ResourceManager.h, font.h, CEO.h
Registry.h:             Platform.h, RuntimeReflect.h
Savestates.h:           Platform.h
StateInterface.h:       Platform.h
ScreenManager.h:        framebuffer.h
TextureManager.h:       texture.h
UIManager.h:            Registry.h, MathLib.h
====================================*/

// ----------------------------
// Engine-Related
// ----------------------------
#ifndef RAPIDJSON_HAS_STDSTRING
    #define RAPIDJSON_HAS_STDSTRING 1
#endif

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "Registry.h"               
#include "MathLib.h"
#include "Profiler.h"               
#include "StateInterface.h"
#include "Camera.h"                 
#include "CEO.h"                    
#include "Errorlog.h"     
#include "EventsDispatcher.h"       
#include "Logger.h"
#include "Savestates.h"
// ----------------------------
// Engine-Related (Managers)
// ----------------------------
#include "FontManager.h"            
#include "AudioManager.h"           
#include "TextureManager.h"        
#include "FileManager.h"        
#include "ScreenManager.h"          
#include "UIManager.h"              
#include "HierarchyManager.h"       
#include "InputManager.h"           
#include "LayerManager.h"
#include "PrefabManager.h"
#include "GameObjects.h"
#include "RenderUtils.h"

// ----------------------------
// Platform-specific
// ----------------------------
#ifdef _WIN32
    #include <winsock2.h>
    #include <Windows.h>
    #undef DELETE
#endif

