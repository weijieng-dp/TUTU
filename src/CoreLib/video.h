/*!
@file       video.h
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) (10%)
@date       25/02/2026

This file declares the [VideoObj] class, which encapsulates video playback
state, resource management, and lifecycle. VideoObj owns the pl_mpeg decoder
instance, GPU texture resources, and CPU-side frame buffer. Move semantics
enable safe resource transfer within the VideoManager cache system.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/


#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "pl_mpeg.h"

class VideoObj{

    public:
    /*!
    * \brief
    *    Default constructor for [VideoObj], initializes all members to default values.
    *
    * \param
    *    [None]
    *
    * \return
    *    [VideoObj] A video object with no loaded video, empty state, and null resources.
    */
    VideoObj();

    // non-copyable
    VideoObj(const VideoObj&) = delete;
    VideoObj& operator=(const VideoObj&) = delete;

    /*!
    * \brief
    *    Move constructor for [VideoObj], transfers resource ownership from another VideoObj.
    *    The source object is left in a valid but empty state after the move.
    *
    * \param
    *    other - rvalue reference to the source VideoObj whose resources will be transferred.
    *
    * \return
    *    [VideoObj] A video object that now owns the resources previously held by other.
    */
    VideoObj(VideoObj&& other) noexcept;

    /*!
    * \brief
    *    Move assignment operator for [VideoObj], replaces this object's resources with
    *    another's using the copy-and-swap idiom. The source object is left in a valid
    *    but empty state after the assignment.
    *
    * \param
    *    other - rvalue reference to the source VideoObj whose resources will be transferred.
    *
    * \return
    *    [VideoObj&] A reference to this object after resource transfer.
    */
    VideoObj& operator=(VideoObj&& other) noexcept;

    /*!
    * \brief
    *    Swaps all resources and state between this VideoObj and another.
    *    Used internally by the move assignment operator to safely exchange ownership.
    *
    * \param
    *    other - reference to the VideoObj to swap with.
    *
    * \return
    *    [void]
    */
    void Swap(VideoObj& other);

    /*!
    * \brief
    *    Destructor for [VideoObj], automatically releases the pl_mpeg decoder and
    *    GPU texture resources. Called when the object goes out of scope or is deleted.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    ~VideoObj();

    /*!
    * \brief
    *    Returns the file path of the loaded video.
    *
    * \param
    *    [None]
    *
    * \return
    *    [const std::string&] The file path used to load this video resource.
    */
    std::string const& GetPath() const { return path; }

    /*!
    * \brief
    *    Returns the width of the video in pixels.
    *
    * \param
    *    [None]
    *
    * \return
    *    [int] The video width (>0 if video is loaded, 0 otherwise).
    */
    int GetWidth() const { return width; }

    /*!
    * \brief
    *    Returns the height of the video in pixels.
    *
    * \param
    *    [None]
    *
    * \return
    *    [int] The video height (>0 if video is loaded, 0 otherwise).
    */
    int GetHeight() const { return height; }

    /*!
    * \brief
    *    Returns the total duration of the video in seconds.
    *
    * \param
    *    [None]
    *
    * \return
    *    [double] The total playback time in seconds (0.0 if video is not loaded).
    */
    double GetDuration() const { return duration; }

    /*!
    * \brief
    *    Returns the current playback time in seconds.
    *
    * \param
    *    [None]
    *
    * \return
    *    [double] The current position within the video (0.0 to duration).
    */
    double GetCurrentTime() const { return currentTime; }

    /*!
    * \brief
    *    Returns whether the video is currently playing.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if playback is active, false if paused or stopped.
    */
    bool IsPlaying() const { return isPlaying; }

    /*!
    * \brief
    *    Returns whether the video has reached the end.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if the video decoder has finished, false otherwise.
    */
    bool HasEnded() const { return hasEnded; }

    /*!
    * \brief
    *    Returns whether an ended event is pending to be consumed by the application.
    *    This is a one-shot flag that is cleared after being read.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if video end event has not yet been consumed by VideoManager.
    */
    bool HasEndedEventPending() const { return endedEventPending; }

    /*!
    * \brief
    *    Returns whether a new frame has been decoded and is ready for upload to GPU.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if a newly decoded frame is available this frame, false otherwise.
    */
    bool HasNewFrame() const { return hasNewFrame; }

    /*!
    * \brief
    *    Returns a const reference to the CPU-side frame buffer containing decoded RGB data.
    *    The buffer is allocated lazily on first decode and contains width * height * 3 bytes.
    *
    * \param
    *    [None]
    *
    * \return
    *    [const std::vector<uint8_t>&] Reference to the RGB frame data buffer.
    */
    std::vector<uint8_t> const& GetFrameBuffer() const { return frameBuffer; }

    /*!
    * \brief
    *    Returns the OpenGL texture handle for the GPU-side frame texture.
    *    The texture is created lazily on first frame upload.
    *
    * \param
    *    [None]
    *
    * \return
    *    [GLuint] The OpenGL texture object ID (0 if not yet created).
    */
    GLuint GetFrameTextureId() const { return frameTextureId; }

    /*!
    * \brief
    *    Returns whether the GPU frame texture is ready for rendering.
    *    This is false until the first frame is successfully uploaded.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] true if texture has been created and populated with frame data.
    */
    bool IsFrameTextureReady() const { return isFrameTextureReady; }

    /*!
    * \brief
    *    Returns the current playback speed multiplier.
    *
    * \param
    *    [None]
    *
    * \return
    *    [double] The playback speed (1.0 = normal, 2.0 = double speed, 0.5 = half speed).
    */
    double GetPlaybackSpeed() const { return playbackSpeed; }


    private:
    
    /*!
    * \brief
    *    Releases any loaded video resource and resets internal pointers.
    *    Destroys the pl_mpeg decoder and deletes the GPU texture if one exists.
    *    Called by the destructor and during resource cleanup.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void Release();


    std::string path{};                     // filepath of the loaded video

    plm_t* plm{ nullptr };                  // pl_mpeg decoder instance (null if not loaded)
    TextureObj texture;                     // wrapper for GPU texture handle


    // ===== Playback State =====
    bool isPlaying{ false };                // whether video is currently playing
    bool isLooping{ false };                // whether video should loop on end
    bool hasEnded{ false };                 // whether decoder has reached end
    bool hasNewFrame{ false };              // whether new frame was decoded this tick
    bool endedEventPending{ false };        // one-shot flag for end-of-video event

    double currentTime{ 0.0 };              // current playback position in seconds
    double duration{ 0.0 };                 // total video duration in seconds
    double frameRate{ 0.0 };                // video frame rate (fps)
    int width{ 0 };                         // video width in pixels
    double playbackSpeed{ 1.0 };            // playback speed multiplier (1.0 = normal)
    int height{ 0 };                        // video height in pixels

    // ===== CPU-side Frame Data =====
    // CPU-side decoded RGB frame, to be uploaded by renderer path.
    // Size = width * height * 3 bytes (RGB format, 8-bit per channel)
    std::vector<uint8_t> frameBuffer{};

    // ===== GPU-side Texture Cache =====
    // GPU texture cache for decoded frames (created lazily on first frame).
    // Reused for all subsequent frame uploads to avoid redundant allocation.
    GLuint frameTextureId{ 0 };             // OpenGL texture object handle
    bool isFrameTextureReady{ false };      // whether texture has been created and populated

    // Grant VideoManager access to private members for direct state management
    friend class VideoManager;
};
