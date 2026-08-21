/*!
@file       VideoTest.h
@author     Zhang Mingyang (mingyang.zhang) (90%)
@co-author  Ou Yukang (yukang.ou) 10%
@date       25/02/2026

This file declares the [VideoTest] class, a native script for testing and
demonstrating video playback functionality. VideoTest encapsulates video
configuration parameters (path, playback speed, looping) and cutscene
orchestration settings (autoplay, skip, scene transitions). It serves as
both a functional test harness and reference implementation for integrating
video playback into gameplay or cutscenes via the VideoComponent system.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class VideoTest : public ScriptInstance
{
public:
    /*!
    * \brief
    *    Binds script data from the registry component to this local script instance.
    *    Retrieves the VideoTest component from the registry and copies its state into
    *    this object, ensuring the script has the latest configuration values.
    *    Called by the scripting system before OnUpdate to sync external changes.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void BindFrom() {
        GetComponent<VideoTest>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<VideoTest>(*CEO::Get<Registry>());
    };

    /*!
    * \brief
    *    Binds this script's local data back to the registry component.
    *    Copies the current state of this object into the VideoTest component stored
    *    in the registry, synchronizing any changes made during script execution.
    *    Called by the scripting system after OnUpdate to persist local changes.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void BindTo() { *GetComponent<VideoTest>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief
    *    Called once when the script is initialized. Sets up video loading, configures
    *    the VideoComponent with playback parameters, and initiates autoplay if enabled.
    *    Establishes the initial video resource and applies all component-level policies
    *    for cutscene orchestration.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    *
    * \return
    *    [void]
    */
    void OnStart(Registry& registry);

    /*!
    * \brief
    *    Called every frame during script execution. Monitors video playback state and
    *    responds to lifecycle events (e.g., video ended). Demonstrates how to detect
    *    and handle video completion through the VideoComponent's endedThisFrame flag.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    dt - delta time in seconds since the last frame.
    * \param
    *    firstframe - true if this is the first update after script initialization.
    *
    * \return
    *    [void]
    */
    void OnUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    Called every fixed timestep for deterministic physics or logic updates.
    *    Not used for video playback which operates on variable timestep (see OnUpdate).
    *    Provided as part of the standard scripting interface.
    *
    * \param
    *    registry - reference to the entity registry for component access.
    * \param
    *    dt - fixed timestep delta in seconds.
    * \param
    *    firstframe - true if this is the first fixed update after script initialization.
    *
    * \return
    *    [void]
    */
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    /*!
    * \brief
    *    File path to the video resource to be loaded and played.
    *    Relative to the Assets directory. Example: "Videos/cutscene.mpeg"
    *
    * \return
    *    [std::string] The video file path.
    */
    std::string videoPath{ "Assets/Videos/bjork-all-is-full-of-love.mpeg" };

    /*!
    * \brief
    *    Determines whether the video should loop when it reaches the end.
    *    If true, the video restarts from the beginning; if false, playback stops.
    *
    * \return
    *    [bool] true to loop video, false for one-shot playback.
    */
    bool loopVideo{ false };

    /*!
    * \brief
    *    Playback speed multiplier for the video. Affects decode rate and frame timing.
    *    Examples: 1.0 = normal speed, 2.0 = double speed (fast-forward),
    *    0.5 = half speed (slow-motion).
    *
    * \return
    *    [float] The playback speed multiplier (0.0 or greater).
    */
    float playbackSpeed{ 1.f };

    /*!
    * \brief
    *    Allows the user to skip the video during playback via input.
    *    If true, clicking the mouse or touching the screen will stop the video
    *    and optionally trigger a scene transition (nextSceneOnSkip).
    *
    * \return
    *    [bool] true to enable skip functionality, false to disable.
    */
    bool allowSkip{ true };

    /*!
    * \brief
    *    The input key code that triggers video skip (Windows platform).
    *    Default is 32 (SPACE key). Used by VideoManager::SyncVideoComponents
    *    to detect skip input on PC builds.
    *
    * \return
    *    [int] The GLFW key code for skip input.
    */
    int skipKey{ 32 }; // default SPACE key

    /*!
    * \brief
    *    Scene name or identifier to load when the video ends naturally (without skip).
    *    If empty, no scene transition occurs on video end. Enables cutscenes to
    *    trigger story progression or level changes.
    *
    * \return
    *    [std::string] The target scene identifier on natural video end.
    */
    std::string nextSceneOnEnd{};

    /*!
    * \brief
    *    Scene name or identifier to load when the user skips the video.
    *    If empty, no scene transition occurs on skip. Allows cutscenes to honor
    *    player intent while still advancing the game state.
    *
    * \return
    *    [std::string] The target scene identifier on video skip.
    */
    std::string nextSceneOnSkip{};

    /*!
    * \brief
    *    File path to an external audio track to synchronize with video playback.
    *    Allows independent audio (e.g., voice-over, music) separate from video codec.
    *    If empty, no external audio is played. Audio is managed by ResourceManager.
    *
    * \return
    *    [std::string] The audio file path relative to Assets directory.
    */
    std::string audioPath{};

    /*!
    * \brief
    *    Audio category or type for the external audio track (e.g., "BGM", "SFX", "Voice").
    *    Used by the ResourceManager to apply category-specific volume and mixing settings.
    *    Default is "BGM" for background music.
    *
    * \return
    *    [std::string] The audio type identifier.
    */
    std::string audioType{ "BGM" };

    /*!
    * \brief
    *    Determines whether the external audio track should loop with the video.
    *    If true, audio restarts when video loops; if false, audio plays once.
    *
    * \return
    *    [bool] true to loop audio, false for one-shot audio playback.
    */
    bool audioLoop{ false };

    /*!
    * \brief
    *    Volume level for the external audio track (0.0 to 1.0 range).
    *    0.0 = silent, 1.0 = maximum volume. Applied by ResourceManager during playback.
    *
    * \return
    *    [float] The audio volume multiplier.
    */
    float audioVolume{ 1.f };

    /*!
    * \brief
    *    Pitch multiplier for the external audio track. Affects playback frequency.
    *    1.0 = normal pitch, 2.0 = octave higher, 0.5 = octave lower.
    *    Applied by the audio system (FMOD) during playback.
    *
    * \return
    *    [float] The audio pitch multiplier (0.0 or greater).
    */
    float audioPitch{ 1.f };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(VideoTest),
    field(videoPath),
    field(loopVideo),
    field(playbackSpeed),
    field(allowSkip),
    field(skipKey),
    field(nextSceneOnEnd),
    field(nextSceneOnSkip),
    field(audioPath),
    field(audioType),
    field(audioLoop),
    field(audioVolume),
    field(audioPitch)

)
