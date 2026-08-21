/*!
@file       AudioManager.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       05/11/2025

This file declares the [AudioManager] class, which serves as the core
interface for initializing, managing, and updating the FMOD sound system.
It provides functions for audio resource loading, system updates, and
shutdown procedures, ensuring proper lifecycle control of the audio engine.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include <string>
#include <unordered_map>
#include "audio.h"

/*!
* \brief
*    The [AudioManager] class serves as the central controller for all audio
*    operations within the engine. It manages initialization, updating, and
*    shutdown of the FMOD sound system while providing interfaces for loading
*    and playing audio files. Designed as a singleton, it ensures a single
*    consistent instance throughout the engine's lifecycle.
*
* \brief
*    Usage:
* \brief
*    - Retrieve the singleton instance using `AudioManager::Instance()`.
* \brief
*    - Call `Init()` at application startup to initialize the FMOD system.
* \brief
*    - Use `LoadAudio(name, loop, stream)` to load and configure sound assets.
* \brief
*    - Invoke `Update()` once per frame to process and mix active audio channels.
* \brief
*    - Call `Shutdown()` during engine termination to properly release all
*      FMOD resources and avoid memory leaks.
*
* \return
*    [AudioManager&] Reference to the global singleton instance used to manage
*    audio initialization, updates, and resource handling.
*/
class AudioManager {

    friend class ResourceManager;
    friend class VideoManager;
    /*!
    * \brief
    *    Initializes the FMOD audio system and prepares it for playback.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    bool Init();

    /*!
    * \brief
    *    Updates the FMOD system each frame to process and mix active sounds.
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
    *    Releases all loaded sounds and properly shuts down the FMOD system.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void Free();

    /*!
    * \brief
    *    Releases the audio store by audioObj
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void FreeAudio(AudioObj& obj);

    /*!
    * \brief
    *    Loads an audio file into memory and prepares it for playback.
    *
    * \param
    *    [const std::string&] path - The file path to the audio asset.
    *
    * \return
    *    [AudioObj] Instance representing the loaded audio clip.
    */
    AudioObj LoadAudio(const std::string& name, std::string type, bool loop = false, bool stream = false);

    /*!
    * \brief
    *    Pauses or resumes all active audio groups and their associated sounds.
    *
    * \param
    *    [bool] pause - True to pause all groups, false to resume them.
    *
    * \return
    *    [void]
    */
    void PauseAll(bool);

    /*!
    * \brief
    *    Pauses or resumes playback for all sounds within a specific audio group.
    *
    * \param
    *    [const std::string&] name - The group identifier.
    * \param
    *    [bool] pause - True to pause the group, false to resume it.
    *
    * \return
    *    [void]
    */
    void PauseGroup(const std::string&, bool);

    /*!
    * \brief
    *    Updates audio properties for a specific group, including volume,
    *    pitch, and mute state.
    *
    * \param
    *    [const std::string&] name - The name of the audio group.
    * \param
    *    [float] vol - The volume to apply to the group (default: 1.0f).
    * \param
    *    [float] pitch - The pitch multiplier for the group (default: 1.0f).
    * \param
    *    [bool] mute - Whether the group should be muted (default: false).
    *
    * \return
    *    [void]
    */
    void SetGroupVars(const std::string& name, float vol = 1.f, float pitch = 1.f, bool mute = false);
    
    /*!
    * \brief
    *    Retrieves the current volume level assigned to a specific audio group.
    *
    * \param
    *    [const std::string&] name - The name of the audio group.
    *
    * \return
    *    [float] The group's volume value.
    */
    float GetGroupVol(const std::string& name);
    
    /*!
    * \brief
    *    Retrieves the current pitch multiplier applied to a specific audio group.
    *
    * \param
    *    [const std::string&] name - The name of the audio group.
    *
    * \return
    *    [float] The group's pitch value.
    */
    float GetGroupPitch(const std::string& name);
    
    /*!
    * \brief
    *    Indicates whether a specific audio group is currently muted.
    *
    * \param
    *    [const std::string&] name - The name of the audio group.
    *
    * \return
    *    [bool] True if the group is muted, false otherwise.
    */
    bool GetGroupMute(const std::string& name);
    
    /*!
    * \brief
    *    Stops all currently playing audio in the system, regardless of group.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void StopAll();

    /*!
* \brief
*   Gradually increases the volume of the given BGM channel from 0 to full
*   volume over a specified duration.
*
* \param [FMOD::Channel*] channel - the FMOD channel playing the background music
* \param [float] duration - the total duration of the fade-in effect
* \param [float] dt - delta time since the previous frame
*
* \return [bool] true if the fade-in has completed, false otherwise
*/
    bool FadeInBGM(FMOD::Channel* channel, float duration,float dt);


    /*!
* \brief
*   Gradually decreases the volume of the given BGM channel from full volume
*   to silence over a specified duration.
*
* \param [FMOD::Channel*] channel - the FMOD channel playing the background music
* \param [float] duration - the total duration of the fade-out effect
* \param [float] dt - delta time since the previous frame
*
* \return [bool] true if the fade-out has completed, false otherwise
*/
    bool FadeOutBGM(FMOD::Channel* channel, float duration, float dt);

public:


    AudioManager() = default;
    ~AudioManager() = default;
private:
    FMOD::System* SoundSystem{nullptr};
    bool initialized{false}; //Boolean flag indicating whether the FMOD system has been successfully initialized.
    FMOD::ChannelGroup* mainGroup{ nullptr };   // The main channelgroup containing everything else
    std::unordered_map<std::string, FMOD::ChannelGroup*> subGroups; // All of the subgroups
    float fadeInTimer = 0.f;
    float fadeOutTimer = 0.f;
    
};