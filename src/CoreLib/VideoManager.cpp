/*!
@file       VideoManager.cpp
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/02/2026

Implementations of the VideoManager class, which manages MPEG1 video playback
and rendering throughout the engine. This manager handles video resource loading
and caching via the pl_mpeg library, frame decoding, GPU texture synchronization,
VideoComponent lifecycle integration, and cutscene orchestration including
autoplay, looping, seeking, playback speed control, and skip policies. Audio
synchronization is provided through the ResourceManager for external audio tracks.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
*/
/*____________________________________________________________________________*/
#include "pch.h"
#include "VideoManager.h"
#include "FileManager.h"
#include "Components.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include <cstdlib>
#include <cstring>
#include <algorithm>

std::unordered_map<std::string, VideoObj> VideoManager::loadedVideos{};

// Initialize manager state
bool VideoManager::Init() {
    if (initialized) return true;

    initialized = true;
    return true;
}

// Legacy manager update entrypoint; prefers Tick(dt) from main loop.
void VideoManager::Update() {
    Tick(1.0 / 60.0);
}

void VideoManager::Free() {
    UnloadAllVideos();
}

void VideoManager::Tick(double dt)
{
    // Clamp dt to non-negative to avoid invalid decode stepping.
    const double safeDt = std::max(0.0, dt);
    for (auto& [path, video] : loadedVideos) {
        (void)path;

        if (!video.plm || !video.isPlaying) {
            continue;
        }

        if (plm_has_ended(video.plm)) {
            if (video.isLooping) {
                // Looping clip: rewind and continue playback from start.
                plm_rewind(video.plm);
                video.currentTime = 0.0;
                video.hasEnded = false;
                video.endedEventPending = false;
            }
            else {
                // Non-looping clip: stop and publish one-shot ended event only if
                // playback actually advanced or produced a frame.
                const bool hasMeaningfulPlayback =
                    (video.currentTime > 0.0) || video.isFrameTextureReady;

                video.isPlaying = false;
                video.hasEnded = true;
                video.endedEventPending = hasMeaningfulPlayback;

                if (!hasMeaningfulPlayback) {
                    LOGE("Video ended before first frame/time advanced: %s", video.path.c_str());
                }
            }
            continue;
        }

        // pl_mpeg pushes decoded frames into our callback.
        video.hasNewFrame = false;

        // Playback speed scales decode delta for fast-forward/slow-mo behavior.
        const double scaledDt = safeDt * std::max(0.0, video.playbackSpeed);
        plm_decode(video.plm, scaledDt);
        video.currentTime = std::min(plm_get_time(video.plm), video.duration);  

        // Upload latest CPU-decoded frame to GPU texture for render usage.
        if (video.hasNewFrame) {
            UploadFrameToTexture(video);
            video.hasNewFrame = false;
        }
    }   
}


bool VideoManager::IsManagedVideo(VideoObj const* video)
{
    if (!video) {
        return false;
    }

    // Validate pointer ownership against manager cache to avoid stale references.
    for (auto const& [path, obj] : loadedVideos) {
        (void)path;
        if (&obj == video) {
            return true;
        }
    }
    return false;
}


void VideoManager::SyncVideoComponents(Registry& registry)
{
    auto stopAudioForComp = [](VideoComponent* comp) {
        if (!comp) return;

        // Stop and clear active cutscene audio channel if one exists.
        if (comp->audioChannel) {
            comp->audioChannel->stop();
            comp->audioChannel = nullptr;
        }
        comp->isAudioStarted = false;
        };


    // Generalized bridge: every VideoComponent can load/play/emit events.
    // Sprite assignment is optional and only applied when a sprite exists.
    auto entities = registry.GetEntitiesWithComponent<VideoComponent>();
    for (EntityRegistry::Entity entity : entities) {
        VideoComponent* videoComp = registry.GetComponent<VideoComponent>(entity);
        if (!videoComp) {
            continue;
        }

        // Reset one-frame event flag; this may be raised again below.
        videoComp->endedThisFrame = false;

        // Runtime/editor validation: sanitize values before applying policies.
        videoComp->playbackSpeed = std::max(0.f, videoComp->playbackSpeed);
        videoComp->audioVolume = std::clamp(videoComp->audioVolume, 0.f, 1.f);
        videoComp->audioPitch = std::max(0.f, videoComp->audioPitch);
        if (videoComp->audioType.empty()) {
            videoComp->audioType = "BGM";
        }

        // Reset stale pointer if it is no longer manager-owned (e.g., after resource reset).
        if (videoComp->video && !IsManagedVideo(videoComp->video)) {
            videoComp->video = nullptr;
            videoComp->boundVideoPath.clear();
            stopAudioForComp(videoComp);
        }

        // Resolve desired video resource by path and switch sources safely.
        if (videoComp->videoPath.empty()) {
            // Empty path means video is intentionally disabled for this component.
            videoComp->video = nullptr;
            videoComp->boundVideoPath.clear();
            videoComp->hasAutoStarted = false;
            videoComp->isEnded = false;
            stopAudioForComp(videoComp);
            continue;
        }

        if (!videoComp->video || videoComp->boundVideoPath != videoComp->videoPath) {
            videoComp->video = GetOrLoadVideo(videoComp->videoPath);
            if (!videoComp->video) {
                continue;
            }

            videoComp->boundVideoPath = videoComp->videoPath;
            videoComp->hasAutoStarted = false;
            videoComp->isEnded = false;
            stopAudioForComp(videoComp);
        }

        // Keep playback speed synced with component value for live tuning in editor/scripts.
        SetPlaybackSpeed(videoComp->video, videoComp->playbackSpeed);

        // One-time autoplay kickoff controlled by component settings.
        if (videoComp->autoplay && !videoComp->hasAutoStarted) {
            RestartVideo(videoComp->video);
            PlayVideo(videoComp->video, videoComp->loop);
            videoComp->hasAutoStarted = true;
            videoComp->isEnded = false;
            // Start external audio track in sync with autoplay kickoff.
            if (!videoComp->audioPath.empty() && !videoComp->isAudioStarted) {
      
                CEO::Get<ResourceManager>()->QueueBGM(videoComp->audioPath);
                videoComp->isAudioStarted = CEO::Get<ResourceManager>()->QueueSize();
            }
        }

        // Component-level skip policy for cutscene flow control.
#ifdef PLATFORM_WINDOWS
        if (videoComp->allowSkip && (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || Input::IsGamepadButtonPressed(Input::GamepadButton::A, 0))) {
#else
        if (videoComp->allowSkip && Input::IsPointerPressed(0)) {
#endif
            StopVideo(videoComp->video);
            stopAudioForComp(videoComp);
            videoComp->endedThisFrame = true;
            videoComp->isEnded = true;

            if (!videoComp->nextSceneOnSkip.empty()) {
                //SceneManager::QueueSceneAction(videoComp->nextSceneOnSkip, SceneManager::CHANGE);
            }
        }

        // If video started externally (e.g. via script), start configured audio on demand.
        if (videoComp->video->isPlaying && !videoComp->isAudioStarted && !videoComp->audioPath.empty()) {
            CEO::Get<ResourceManager>()->QueueBGM(videoComp->audioPath);

            videoComp->isAudioStarted = CEO::Get<ResourceManager>()->QueueSize();
        }

        // Keep audio lifecycle aligned if playback was stopped externally.
        if (!videoComp->video->isPlaying && videoComp->isAudioStarted) {
            stopAudioForComp(videoComp);
        }


        // Propagate one-shot ended event to component state for script/gameplay consumers.
        if (ConsumeEndedEvent(videoComp->video)) {
            videoComp->endedThisFrame = true;
            videoComp->isEnded = true;
            stopAudioForComp(videoComp);

            // Optional end-of-video scene transition policy.
            if (!videoComp->nextSceneOnEnd.empty()) {
                //SceneManager::QueueSceneAction(videoComp->nextSceneOnEnd, SceneManager::CHANGE);
            }
        }

        // Sprite assignment is optional; skip entities without SpriteRenderer.
        SpriteRendererComponent* sprite = registry.GetComponent<SpriteRendererComponent>(entity);
        if (!sprite) {
            continue;
        }

        TextureObj* videoTexture = GetRenderTexture(videoComp->video);
        if (videoTexture) {
            sprite->texture = videoTexture;
        }
    }
}

void AndroidMakesMeMad(std::string& s) {
    std::replace(s.begin(), s.end(), '\\', '/');
}

std::optional<VideoObj> VideoManager::DeserializeVideo(std::string const& filepath)
{
#ifdef PLATFORM_WINDOWS
    std::stringstream bruh = CEO::Instance().GetManager<FileManager>()->ReadFile("Assets\\" + filepath, false, std::ios::binary | std::ios_base::in);
#else
    std::string bruh2 = filepath;
    AndroidMakesMeMad(bruh2);
    std::stringstream bruh = CEO::Instance().GetManager<FileManager>()->ReadFile(bruh2, false, std::ios::binary | std::ios_base::in);
#endif
    std::string const data = bruh.str();
    if (data.empty())
        return std::nullopt;


    uint8_t* buffer = (uint8_t*)(malloc(data.size()));
    if (!buffer) {
        LOGE("Out of memory while loading video: %s", filepath.c_str());
        return std::nullopt;
    }

    std::memcpy(buffer, data.data(), data.size());




    plm_t* plm = plm_create_with_memory(
        buffer,data.size(),1
    );



    if (!plm) { // failed to deserialize data
        //std::free(buffer);
        return std::nullopt;
    }

    // decode video only
    plm_set_video_enabled(plm, 1);
    plm_set_audio_enabled(plm, 0);


    // construct video object
    VideoObj video;
    video.path = filepath;
    video.plm = plm;
    video.width = plm_get_width(plm);
    video.height = plm_get_height(plm);
    video.duration = plm_get_duration(plm);
    video.frameRate = plm_get_framerate(plm);

    if (video.width > 0 && video.height > 0) {
        video.frameBuffer.resize(static_cast<size_t>(video.width) * static_cast<size_t>(video.height) * 3);
    }


    return std::make_optional(std::move(video));
}

VideoObj* VideoManager::GetOrLoadVideo(std::string const& filepath)
{
    auto found = loadedVideos.find(filepath);
    if (found != loadedVideos.end()) {
        return &found->second;
    }

    auto videoOpt = DeserializeVideo(filepath);
    if (!videoOpt) {
        LOGE("Failed to load video: %s", filepath.c_str());
        return nullptr;
    }

    auto [it, inserted] = loadedVideos.emplace(filepath, std::move(*videoOpt));
    if (!inserted) {
        return nullptr;
    }

    VideoObj& video = it->second;
    plm_set_video_decode_callback(video.plm, &VideoManager::OnVideoFrameDecoded, &video);

    return &video;

}


void VideoManager::UnloadAllVideos()
{
    loadedVideos.clear();
}


bool VideoManager::PlayVideo(VideoObj* video, bool loop)
{
    if (!video || !video->plm) {
        return false;
    }

    // Ensure deterministic restart from the beginning when replaying a clip
    // that may have already reached EOF in the shared cache.
    if (video->hasEnded || plm_has_ended(video->plm)) {
        plm_rewind(video->plm);
        video->currentTime = 0.0;
        video->hasNewFrame = false;
    }

    video->isPlaying = true;
    video->isLooping = loop;
    video->hasEnded = false;
    video->endedEventPending = false;
    return true;
}


void VideoManager::PauseVideo(VideoObj* video)
{
    if (!video) {
        return;
    }

    video->isPlaying = false;
}


void VideoManager::StopVideo(VideoObj* video)
{
    if (!video || !video->plm) {
        return;
    }

    video->isPlaying = false;
    video->isLooping = false;
    video->hasEnded = false;
    video->endedEventPending = false;
    video->currentTime = 0.0;
    video->hasNewFrame = false;
    plm_rewind(video->plm);
}



bool VideoManager::SeekVideo(VideoObj* video, double seconds, bool seekExact)
{
    if (!video || !video->plm) {
        return false;
    }

    // Keep seek time in valid clip range.
    const double clamped = std::clamp(seconds, 0.0, video->duration);
    if (!plm_seek(video->plm, clamped, seekExact ? 1 : 0)) {
        return false;
    }

    video->currentTime = plm_get_time(video->plm);
    video->hasEnded = false;
    video->endedEventPending = false;
    return true;
}

bool VideoManager::RestartVideo(VideoObj* video)
{
    if (!video || !video->plm) {
        return false;
    }

    plm_rewind(video->plm);
    video->currentTime = 0.0;
    video->hasEnded = false;
    video->endedEventPending = false;
    return true;
}

void VideoManager::SetPlaybackSpeed(VideoObj* video, double speed)
{
    if (!video) {
        return;
    }

    // Negative speed is not supported by pl_mpeg; clamp at 0.
    video->playbackSpeed = std::max(0.0, speed);
}

bool VideoManager::ConsumeEndedEvent(VideoObj* video)
{
    if (!video || !video->endedEventPending) {
        return false;
    }

    video->endedEventPending = false;
    return true;
}


TextureObj* VideoManager::GetRenderTexture(VideoObj* video)
{
    // Render path should only consume texture after first successful upload.
    if (!video || !video->isFrameTextureReady || video->frameTextureId == 0) {
        return nullptr;
    }

    return &video->texture;
}


void VideoManager::UploadFrameToTexture(VideoObj& video)
{
    if (video.frameBuffer.empty() || video.width <= 0 || video.height <= 0) {
        return;
    }

    // Create GPU texture lazily and reuse it for every decoded frame.
    if (video.frameTextureId == 0) {
        glGenTextures(1, &video.frameTextureId);
        glBindTexture(GL_TEXTURE_2D, video.frameTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB,
            video.width,
            video.height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            video.frameBuffer.data()
        );
        glBindTexture(GL_TEXTURE_2D, 0);

        // Mirror raw GL texture in TextureObj so existing renderers can consume it.
        video.texture = TextureObj(video.frameTextureId, video.width, video.height, GL_FALSE, GL_TRUE);

        video.isFrameTextureReady = true;
        return;
    }

    glBindTexture(GL_TEXTURE_2D, video.frameTextureId);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        video.width,
        video.height,
        GL_RGB,
        GL_UNSIGNED_BYTE,
       video.frameBuffer.data()
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    // Keep render-facing metadata in sync (same GL id, current dimensions).
    video.texture = TextureObj(video.frameTextureId, video.width, video.height, GL_FALSE, GL_TRUE);

    video.isFrameTextureReady = true;
}


void VideoManager::OnVideoFrameDecoded(plm_t*, plm_frame_t* frame, void* user)
{
    if (!frame || !user) {
        return;
    }

    VideoObj* video = static_cast<VideoObj*>(user);
    if (video->frameBuffer.empty()) {
        return;
    }

    const int stride = video->width * 3;
    plm_frame_to_rgb(frame, video->frameBuffer.data(), stride);
    video->hasNewFrame = true;
}
