/*!
@file       pl_mpeg_impl.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       21/01/2026

Single translation unit implementation file for the pl_mpeg library, a MPEG1
video decoder. This file must exist in exactly one translation unit within
the project to provide the complete pl_mpeg implementation. The define
PL_MPEG_IMPLEMENTATION enables the function implementations from pl_mpeg.h,
allowing video playback functionality throughout the engine.

This file serves as the bridge between the engine's video subsystem
(VideoManager and video.cpp) and the underlying MPEG1 decoding library.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
// IMPORTANT: pl_mpeg implementation must exist in exactly one translation unit.
#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"