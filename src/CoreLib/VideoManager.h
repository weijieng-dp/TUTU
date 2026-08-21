/*!
@file       VideoManager.h
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/02/2026

This file declares the [VideoManager] class, which provides centralized management
of video resource loading, caching, decoding, and playback orchestration. VideoManager
operates as a singleton via static methods and integrates with the engine's main loop
via Tick(dt). It decodes MPEG1 video using pl_mpeg, uploads frames to GPU textures,
synchronizes VideoComponent state each frame, and handles audio synchronization through
the ResourceManager.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include <optional>
#include <string>
#include <unordered_map>
#include "video.h"
#include "Registry.h"

class VideoManager {

    friend class ResourceManager;

    /*!
    * \brief
    *    Initializes video manager state. Called once during engine startup.
    *    Idempotent: safe to call multiple times.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if initialization succeeded or was already complete, false otherwise.
    */
    bool Init();

    /*!
    * \brief
    *    Updates active video resources each frame. Legacy update entrypoint that
    *    delegates to Tick(1.0/60.0) for backward compatibility. Prefer calling
    *    Tick(dt) directly from the main loop with actual delta time.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void Update();

    /*!
    * \brief
    *    Releases all loaded video resources and clears the cache.
    *    Called during engine shutdown. All VideoObj pointers become invalid after this.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void Free();

public:
    /*!
    * \brief
    *    Default constructor for [VideoManager]. Creates a manager instance with
    *    uninitialized state. Typically created as a managed resource by the engine.
    *
    * \param
    *    [None]
    *
    * \return
    *    [VideoManager] A video manager instance with no loaded videos.
    */
    VideoManager() = default;

    /*!
    * \brief
    *    Destructor for [VideoManager]. Does not free resources; Free() must be
    *    called explicitly during shutdown to release cached videos.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    ~VideoManager() = default;

    /*!
    * \brief
    *    Deserializes a video file from disk into a VideoObj. Loads the file from
    *    the filesystem, creates a pl_mpeg decoder, and initializes the VideoObj
    *    with video properties (width, height, duration, framerate).
    *
    * \param
    *    filepath - the path to the video file relative to the Assets directory.
    *
    * \return
    *    [std::optional<VideoObj>] A VideoObj containing the loaded decoder and metadata
    *    on success, or std::nullopt if the file could not be loaded or parsed.
    */
    static std::optional<VideoObj> DeserializeVideo(std::string const& filepath);

    /*!
    * \brief
    *    Gets or loads a video resource by filepath. Checks the internal cache first;
    *    if not found, deserializes the file and caches the result. Subsequent calls
    *    with the same filepath return the cached instance (fast path).
    *
    * \param
    *    filepath - the path to the video file relative to the Assets directory.
    *
    * \return
    *    [VideoObj*] A pointer to the cached VideoObj on success, or nullptr if
    *    the file could not be loaded or if the cache insertion failed.
    */
    static VideoObj* GetOrLoadVideo(std::string const& filepath);

    /*!
    * \brief
    *    Unloads all cached videos and clears the resource cache.
    *    All VideoObj pointers held by components become invalid after this call.
    *    Typically called during resource reset or engine shutdown.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    static void UnloadAllVideos();

    /*!
    * \brief
    *    Starts playback of a video. If the video has already ended or been played,
    *    rewinds to the beginning before starting. Safe to call multiple times.
    *
    * \param
    *    video - pointer to the VideoObj to play. Must be a managed video.
    * \param
    *    loop - whether the video should loop when it reaches the end.
    *
    * \return
    *    [bool] true if playback started successfully, false if video pointer is invalid.
    */
    static bool PlayVideo(VideoObj* video, bool loop = false);

    /*!
    * \brief
    *    Pauses playback of a video without resetting state. The video can be resumed
    *    by calling PlayVideo() again.
    *
    * \param
    *    video - pointer to the VideoObj to pause.
    *
    * \return
    *    [void]
    */
    static void PauseVideo(VideoObj* video);

    /*!
    * \brief
    *    Stops playback of a video and resets its state. Rewinds to the beginning,
    *    clears playback flags, and stops looping. The video must be explicitly
    *    played again with PlayVideo().
    *
    * \param
    *    video - pointer to the VideoObj to stop.
    *
    * \return
    *    [void]
    */
    static void StopVideo(VideoObj* video);

    /*!
    * \brief
    *    Seeks the video to a specific time in seconds. The seek time is clamped
    *    to the valid range [0.0, duration]. Clearing ended flags allows playback
    *    to continue after seeking past the end.
    *
    * \param
    *    video - pointer to the VideoObj to seek.
    * \param
    *    seconds - target playback time in seconds.
    * \param
    *    seekExact - if true, seek to exact frame; if false, seek to nearest keyframe
    *               (faster but less precise).
    *
    * \return
    *    [bool] true if seek succeeded, false if video pointer is invalid or seek failed.
    */
    static bool SeekVideo(VideoObj* video, double seconds, bool seekExact = false);

    /*!
    * \brief
    *    Rewinds a video to the beginning without playing it. Useful for resetting
    *    a paused or stopped video back to frame 0.
    *
    * \param
    *    video - pointer to the VideoObj to restart.
    *
    * \return
    *    [bool] true if restart succeeded, false if video pointer is invalid.
    */
    static bool RestartVideo(VideoObj* video);

    /*!
    * \brief
    *    Sets the playback speed multiplier for a video. Speed is clamped to [0.0, ?).
    *    Examples: 1.0 = normal, 2.0 = 2x speed (fast-forward), 0.5 = 0.5x speed (slow-mo).
    *
    * \param
    *    video - pointer to the VideoObj to adjust.
    * \param
    *    speed - playback speed multiplier (0.0 = pause, 1.0 = normal).
    *
    * \return
    *    [void]
    */
    static void SetPlaybackSpeed(VideoObj* video, double speed);

    /*!
    * \brief
    *    Consumes the one-shot ended event for a video. Returns true if the event was
    *    pending, and clears the pending flag. Typically called by VideoManager::SyncVideoComponents
    *    to propagate events to VideoComponent state.
    *
    * \param
    *    video - pointer to the VideoObj to query.
    *
    * \return
    *    [bool] true if an ended event was pending and has now been consumed, false otherwise.
    */
    static bool ConsumeEndedEvent(VideoObj* video);

    /*!
    * \brief
    *    Gets the GPU texture for a decoded video frame. Returns the TextureObj only
    *    if the frame texture has been successfully created and uploaded. Used by
    *    render systems to bind decoded video frames to sprites.
    *
    * \param
    *    video - pointer to the VideoObj to query.
    *
    * \return
    *    [TextureObj*] Pointer to the TextureObj containing the GPU frame texture on success,
    *    or nullptr if the texture is not yet ready (before first frame decode/upload).
    */
    static TextureObj* GetRenderTexture(VideoObj* video);

    /*!
    * \brief
    *    Main update loop for video playback and decoding. Called once per frame with
    *    actual delta time from the engine's main loop. Decodes frames for all playing
    *    videos, uploads new frames to GPU textures, handles looping/ended events, and
    *    applies playback speed scaling.
    *
    * \param
    *    dt - delta time in seconds since the last frame (clamped to non-negative).
    *
    * \return
    *    [void]
    */
    static void Tick(double dt);

    /*!
    * \brief
    *    Synchronizes all VideoComponent instances with their associated VideoObj resources
    *    and updates playback orchestration. Called once per frame after Tick(dt).
    *    Handles resource loading, component state validation, autoplay kickoff, skip input,
    *    audio synchronization, event propagation, and sprite texture binding. Also validates
    *    component pointers against the managed video cache to detect stale references.
    *
    * \param
    *    registry - reference to the entity registry containing all VideoComponent instances.
    *
    * \return
    *    [void]
    */
    static void SyncVideoComponents(Registry& registry);


private:
    /*!
    * \brief
    *    Callback function invoked by pl_mpeg when a video frame is decoded.
    *    Converts the decoded YUV frame to RGB format and stores it in the VideoObj's
    *    CPU-side frame buffer for later GPU upload.
    *
    * \param
    *    [unnamed] - pl_mpeg decoder instance (unused).
    * \param
    *    frame - decoded frame data from pl_mpeg in YUV format.
    * \param
    *    user - user-supplied context pointer (VideoObj* passed to pl_mpeg).
    *
    * \return
    *    [void]
    */
    static void OnVideoFrameDecoded(plm_t* /*plm*/, plm_frame_t* frame, void* user);

    /*!
    * \brief
    *    Uploads a decoded video frame from the CPU-side RGB buffer to a GPU texture.
    *    Creates the texture lazily on first upload, then reuses it for subsequent frames.
    *    Must be called from a thread where the OpenGL context is active.
    *
    * \param
    *    video - reference to the VideoObj containing the frame buffer to upload.
    *
    * \return
    *    [void]
    */
    static void UploadFrameToTexture(VideoObj& video);

    /*!
    * \brief
    *    Validates whether a VideoObj pointer is owned by the manager's cache.
    *    Used to detect stale or invalid pointers held by components after resource resets.
    *
    * \param
    *    video - pointer to validate against the cache.
    *
    * \return
    *    [bool] true if the pointer is a valid managed video, false otherwise (invalid, stale, or external).
    */
    static bool IsManagedVideo(VideoObj const* video);

    /*!
    * \brief
    *    Initialization state flag. Set to true after Init() succeeds.
    *    Used to prevent redundant initialization.
    *
    * \return
    *    [bool] true if manager has been initialized, false otherwise.
    */
    bool initialized{ false };

    /*!
    * \brief
    *    Static cache mapping file paths to loaded VideoObj instances.
    *    Enables resource sharing across multiple VideoComponent instances
    *    and provides fast lookup on repeated requests for the same video.
    *    Cleared by UnloadAllVideos() and Free().
    *
    * \return
    *    [std::unordered_map<std::string, VideoObj>] Map of filepath -> VideoObj.
    */
    static std::unordered_map<std::string, VideoObj> loadedVideos;
};
