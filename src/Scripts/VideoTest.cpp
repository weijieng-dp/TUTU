/*!
@file       VideoTest.cpp
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/02/2026

This file implements the VideoTest class, a test script for validating video
playback functionality. VideoTest demonstrates how to configure VideoComponent
with various playback policies (autoplay, looping, skipping) and how to respond
to video lifecycle events (ended, skipped). It serves as both a functional test
and reference implementation for video orchestration in gameplay or cutscenes.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*____________________________________________________________________________*/
#include "VideoTest.h"
#include "../CoreLib/VideoManager.h"


void VideoTest::OnStart(Registry& r)
{
	// Get the VideoComponent attached to this script's entity
	VideoComponent* videoComp = r.GetComponent<VideoComponent>(entity);
	if (!videoComp) {
		LOGE("VideoTest missing VideoComponent on entity %d", entity);
		return;
	}

	// Load or retrieve cached video from manager
	// GetOrLoadVideo caches the result, so repeated calls are fast
	videoComp->video = VideoManager::GetOrLoadVideo(videoPath);
	if (!videoComp->video) {
		LOGE("VideoTest failed to load video: %s", videoPath.c_str());
		return;
	}

	// Configure component-driven playback/orchestration policy.
	// These settings drive VideoManager::SyncVideoComponents behavior each frame
	videoComp->videoPath = videoPath;
	videoComp->autoplay = true;           // Start playback immediately
	videoComp->loop = loopVideo;          // Enable/disable looping
	videoComp->playbackSpeed = playbackSpeed;  // Speed multiplier (1.0 = normal)
	videoComp->allowSkip = allowSkip;     // Allow user to skip video
	videoComp->skipKey = skipKey;         // Input key for skipping (default SPACE)
	videoComp->nextSceneOnEnd = nextSceneOnEnd;      // Scene to load on natural end
	videoComp->nextSceneOnSkip = nextSceneOnSkip;    // Scene to load on skip

	// Configure external audio track synchronization
	// Allows for separate audio (e.g., voice-over) independent of video codec
	videoComp->audioPath = audioPath;     // Path to audio file
	videoComp->audioType = audioType;     // Audio category (e.g., "BGM", "SFX")
	videoComp->audioLoop = audioLoop;     // Loop audio with video
	videoComp->audioVolume = audioVolume; // Audio volume (0.0 = silent, 1.0 = max)
	videoComp->audioPitch = audioPitch;   // Audio pitch modifier (1.0 = normal)

	// Apply playback speed to the underlying pl_mpeg decoder
	// Must be called after assigning the video pointer
	VideoManager::SetPlaybackSpeed(videoComp->video, playbackSpeed);
	
	// Start video playback with specified loop setting
	// Will be overridden by SyncVideoComponents if autoplay is true
	VideoManager::PlayVideo(videoComp->video, loopVideo);
};


void VideoTest::OnUpdate(Registry& registry, float dt, bool firstframe)
{
	(void)dt;           // Suppress unused parameter warning
	(void)firstframe;   // Suppress unused parameter warning
	
	// Orchestration policy is now handled by VideoManager::SyncVideoComponents.
	// This script only needs to react to state changes; no active management required.
	
	VideoComponent* videoComp = registry.GetComponent<VideoComponent>(entity);
	if (!videoComp || !videoComp->video) {
		return;	
	}

	// Check if video ended this frame (one-shot event)
	// endedThisFrame is only true for a single frame when video completes
	if (videoComp->endedThisFrame) {
		LOGI("VideoTest detected video end on entity %d", entity);
		// Custom logic can be added here to respond to video completion
		// Example: spawn particles, trigger animation, etc.
	}
};

void VideoTest::OnFixedUpdate(Registry&, float, bool)
{
	// Fixed timestep update (physics, deterministic logic)
	// Not used for video playback which operates on variable timestep
};
