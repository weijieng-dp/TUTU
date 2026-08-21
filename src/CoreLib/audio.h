/*!
@file       audio.h
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       05/11/2025

This file declares the [AudioObj] class, which encapsulates audio playback
functionality using the FMOD sound system. It provides methods for playing,
pausing, stopping, and adjusting audio properties such as volume and looping
state for individual sound instances.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/


#pragma once
#ifdef PLATFORM_ANDROID
#include "fmod_android.h"
#include "fmod.hpp"
#else
#include "fmod.hpp"
#endif
#include <string>

/*!
* \brief
*    The [AudioObj] class represents a single sound object managed by the FMOD
*    system. It encapsulates all data and behavior required to play, control, and
*    manage the lifetime of a specific sound instance, including playback state,
*    system reference, and associated FMOD sound data.
*
* \brief
*    Usage:
* \brief
*    - Construct an instance using a valid `FMOD::Sound*` and `FMOD::System*`.
* \brief
*    - Use `Play(volume, pitch)` to start playback or resume a paused sound.
* \brief
*    - Query `IsActive()` to verify if the sound resource is valid and active.
* \brief
*    - The object automatically releases its resources on destruction to ensure
*      proper cleanup of FMOD handles.
*
* \return
*    [AudioObj] Represents an FMOD sound instance capable of playback and control.
*/
class AudioObj{

    public:
    AudioObj() = default;

    /*!
    * \brief
    *    Constructs an [AudioObj] using a valid FMOD sound and system pointer.
    *    Automatically sets the active state based on initialization success.
    *
    * \param
    *    [FMOD::Sound*] s - Pointer to the FMOD sound resource.
    * \param
    *    [FMOD::System*] sys - Pointer to the FMOD audio system instance.
    *
    * \return
    *    [AudioObj] Active audio object bound to FMOD.
    */
    AudioObj(FMOD::Sound* s, FMOD::System* sys, FMOD::ChannelGroup* grp, std::string type, std::string path) : sound{ s }, system{ sys }, isActive{ s && sys }, parent{ grp }, type{ type }, path{ path } {}

    // non-copyable
    AudioObj(const AudioObj&) = delete;
    AudioObj& operator=(const AudioObj&) = delete;

    // move operators
    AudioObj(AudioObj&& other) noexcept;
    AudioObj& operator=(AudioObj&& other) noexcept;

    //dtor
    ~AudioObj() = default;

    /*!
    * \brief
    *    Plays the audio clip from the beginning or resumes playback if paused.
    *
    * \param [float] volume - The playback volume (range: 0.0f to 1.0f). This is scaled against it's parent group's base volume.
    * \param [float] pitch - The playback pitch (range: 0.0f to 1.0f). This is scaled against it's parent group's base pitch.
    *
    * \return
    *    [void]
    */
    FMOD::Channel* Play(float volume = 1.f, float pitch = 1.f, float pan = 0.0f);

    /*!
    * \brief
    *    Returns whether the audio object is currently active and valid for playback.
    *
    * \param
    *    [None]
    *
    * \return
    *    [bool] True if the object is initialized and ready for playback, false otherwise.
    */
    bool IsActive() const noexcept { return isActive; }

	std::string const& GetPath() { return path; }

    private:
    
    void Swap(AudioObj& other) noexcept;

    /*!
    * \brief
    *    Releases any loaded FMOD sound resource and resets internal pointers.
    *
    * \param
    *    [None]
    *
    * \return
    *    [void]
    */
    void Release();


    FMOD::Sound* sound{nullptr};
    FMOD::System* system{nullptr};
    FMOD::ChannelGroup* parent{ nullptr };  // this will be either bgm or sfx
    std::string type{ 0 };                  // stores the name
    std::string path{};                     // filepath
    bool isActive{false};                   // Indicates whether the audio object is valid and active within the FMOD system.

    friend class AudioManager;
};
