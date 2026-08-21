/*!
@file       video.cpp
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/02/2026

This file implements the [VideoObj] class, which encapsulates video playback
state and resource management. VideoObj owns the pl_mpeg decoder instance,
GPU texture resources, and frame buffer data. Move semantics are implemented
to safely transfer ownership between cache entries and component instances.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "video.h"

// Default constructor: initializes VideoObj with default-constructed members
// Members are zero-initialized by their respective constructors
VideoObj::VideoObj()
{
	// All members use default initialization (see video.h for default values)
}

// Move constructor: transfers ownership of resources from other to this
// This is critical for cache operations where videos move between containers
VideoObj::VideoObj(VideoObj&& other) noexcept :
	plm{ other.plm },										// Take ownership of pl_mpeg decoder
	texture{ std::move(other.texture) },				// Move TextureObj (may be empty)
	path{ std::move(other.path) },					// Move file path string
	isPlaying{ other.isPlaying },							// Copy playback state
	isLooping{ other.isLooping },							// Copy loop flag
	hasEnded{ other.hasEnded },								// Copy end flag
	hasNewFrame{ other.hasNewFrame },						// Copy frame availability flag
	endedEventPending{ other.endedEventPending },			// Copy event pending flag
	currentTime{ other.currentTime },						// Copy current playback time
	duration{ other.duration },								// Copy total video duration
	frameRate{ other.frameRate },							// Copy video frame rate (fps)
	width{ other.width },									// Copy video width in pixels
	playbackSpeed{ other.playbackSpeed },					// Copy playback speed multiplier
	height{ other.height },									// Copy video height in pixels
	frameBuffer{ std::move(other.frameBuffer) },		// Move frame RGB buffer
	frameTextureId{ other.frameTextureId },					// Take ownership of GL texture handle
	isFrameTextureReady{ other.isFrameTextureReady }		// Copy texture readiness flag
{
	// Nullify and reset source object to prevent double-delete on its destruction
	// This is the key to safe move semantics: source becomes "empty"
	other.plm = nullptr;
	other.isPlaying = false;
	other.isLooping = false;
	other.hasEnded = false;
	other.hasNewFrame = false;
	other.endedEventPending = false;
	other.currentTime = 0.0;
	other.duration = 0.0;
	other.frameRate = 0.0;
	other.width = 0;
	other.playbackSpeed = 1.0;
	other.height = 0;
	other.frameTextureId = 0;
	other.isFrameTextureReady = false;
	// Note: frameBuffer and texture are already moved (nullified by move semantics)
}


// Move assignment operator: replaces this object's resources with other's
// Uses copy-and-swap idiom for exception safety
VideoObj& VideoObj::operator=(VideoObj&& other) noexcept
{
	// Create temporary object by moving from other
	// This ensures we don't lose resources if something goes wrong
	VideoObj copy{ std::move(other) };
	// Swap this with the temporary (now contains other's resources)
	Swap(copy);
	// copy goes out of scope and cleans up this's old resources
	return *this;
}

// Swap function: exchanges all resources between two VideoObj instances
// Used by move assignment to safely transfer ownership
void VideoObj::Swap(VideoObj& other)
{
	// Swap pl_mpeg decoder ownership
	std::swap(plm, other.plm);
	// Swap GPU texture wrapper
	std::swap(texture, other.texture);
	// Swap video file path
	std::swap(path, other.path);
	
	// Swap playback state flags
	std::swap(isPlaying, other.isPlaying);
	std::swap(isLooping, other.isLooping);
	std::swap(hasEnded, other.hasEnded);
	std::swap(hasNewFrame, other.hasNewFrame);
	std::swap(endedEventPending, other.endedEventPending);
	
	// Swap timing information
	std::swap(currentTime, other.currentTime);
	std::swap(duration, other.duration);
	std::swap(frameRate, other.frameRate);
	
	// Swap video properties
	std::swap(width, other.width);
	std::swap(playbackSpeed, other.playbackSpeed);
	std::swap(height, other.height);
	
	// Swap frame buffer (CPU-side decoded RGB data)
	std::swap(frameBuffer, other.frameBuffer);
	
	// Swap GPU texture handle and readiness flag
	std::swap(frameTextureId, other.frameTextureId);
	std::swap(isFrameTextureReady, other.isFrameTextureReady);
}


// Destructor: automatically called when VideoObj is destroyed
// Ensures all resources (pl_mpeg decoder and GL textures) are properly released
VideoObj::~VideoObj()
{
	Release();
}

// Manual resource cleanup function
// Called by destructor and can be called explicitly if needed
void VideoObj::Release() {
	// Destroy pl_mpeg decoder and free associated memory
	// This also frees the buffer that was allocated in VideoManager::DeserializeVideo
	if(plm) plm_destroy(plm);
	plm = nullptr;

	// Clean up cached frame texture if we created one for decoded output.
	// Must be done on the thread where the GL context is active
	if (frameTextureId != 0) {
		glDeleteTextures(1, &frameTextureId);
		frameTextureId = 0;
		isFrameTextureReady = false;
	}
}

