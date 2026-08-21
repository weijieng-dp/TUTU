/*!
@file       ScreenManager.h
@author     Ou Yukang (yukang.ou) 100%
@date       04/11/2025
@brief		Holds the screen buffer and handles operations that might relate
			to the screen such as post processing and screen size changes


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "FrameBuffer.h"

class ScreenManager {
	ScreenManager(const ScreenManager&) = delete;
	ScreenManager& operator=(const ScreenManager&) = delete;

public:
	ScreenManager() = default;
	~ScreenManager() = default;


	/*!
	* \brief
	*	initialize the ScreenManager, notably the screenbuffer
	*	where all screen contents are rendered to
	*/
	void static Init(GLint width, GLint height, GLint minimapWidth = 256, GLint minimapHeight = 256);
	/*!
	* \brief
	*	resize all frame buffers 
	*/
	void static ResizeBuffers(GLint width, GLint height);
	
	/*!
	* \brief
	*	release any resources used by ScreenManager
	*/
	void static Free();

	static FrameBuffer screenBuffer;	// final output buffer
	static FrameBuffer brightBuffer;	// bloom "bright" pixel buffer
	static std::array<FrameBuffer, 2> postProcessBuffers; // buffer used for intermediate steps in post 
	
	static FrameBuffer minimapScreenBuffer;			// screen buffer for minimap (moving components like the entities)
	static FrameBuffer minimapStaticScreenBuffer;	// screen buffer for minimap (the tilemap background)
};