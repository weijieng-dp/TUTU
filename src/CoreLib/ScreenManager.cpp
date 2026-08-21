/*!
@file       ScreenManager.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       04/11/2025
@brief		Holds the screen buffer and handles operations that might relate
			to the screen such as post processing and screen size changes


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "ScreenManager.h"
#include "Components.h"

FrameBuffer ScreenManager::screenBuffer;
FrameBuffer ScreenManager::brightBuffer;
std::array<FrameBuffer, 2> ScreenManager::postProcessBuffers;

FrameBuffer ScreenManager::minimapScreenBuffer;
FrameBuffer ScreenManager::minimapStaticScreenBuffer;

void ScreenManager::Init(GLint width, GLint height, GLint minimapWidth, GLint minimapHeight ) {
	screenBuffer.Init(width, height, FrameBuffer::ATTACH_COLOR | FrameBuffer::ATTACH_ENTITY_ID | FrameBuffer::ATTACH_DEPTH | FrameBuffer::ATTACH_ADDITIONAL_COLOR);
	brightBuffer.Init(width, height, FrameBuffer::ATTACH_COLOR);
	for (int i = 0; i < postProcessBuffers.size(); ++i)
		postProcessBuffers[i].Init(width, height, FrameBuffer::ATTACH_COLOR | FrameBuffer::ATTACH_LIGHT, true);

	minimapScreenBuffer.Init(minimapWidth, minimapHeight, FrameBuffer::ATTACH_COLOR | FrameBuffer::ATTACH_DEPTH, true);
	minimapStaticScreenBuffer.Init(minimapWidth, minimapHeight, FrameBuffer::ATTACH_COLOR | FrameBuffer::ATTACH_DEPTH, true);
}
void ScreenManager::ResizeBuffers(GLint width, GLint height)
{
	screenBuffer.Resize(width, height);
	brightBuffer.Resize(width, height);

	for (int i = 0; i < postProcessBuffers.size(); ++i)
		postProcessBuffers[i].Resize(width, height);
}

void ScreenManager::Free() {
	screenBuffer.Free();
	brightBuffer.Free();
	for (int i = 0; i < postProcessBuffers.size(); ++i)
		postProcessBuffers[i].Free();
	minimapScreenBuffer.Free();
	minimapStaticScreenBuffer.Free();
}

