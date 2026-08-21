/*!
@file       AudioManager.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       05/11/2025

This file implements the [AudioManager] class, responsible for managing
the initialization and lifecycle of the FMOD audio system. It handles
audio loading, system updates, and shutdown, serving as a centralized
controller for all sound playback and resource management operations.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "AudioManager.h"


inline float LerpClamped(float a, float b, float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return a + (b - a) * t;
}


// Create the FMOD system object and initialize it with 32 channels
bool AudioManager::Init() {
    if (initialized) return true;

    FMOD_RESULT r = FMOD::System_Create(&SoundSystem);
    if (r != FMOD_OK) {
        return false;
    }
    SoundSystem->setOutput(FMOD_OUTPUTTYPE_AUDIOTRACK );
    r = SoundSystem->init(32, FMOD_INIT_NORMAL, nullptr);
    if (r != FMOD_OK) {
        SoundSystem->release();
        SoundSystem = nullptr;
        return false; //prevent leaks if initialization fails
    }
    r = SoundSystem->createChannelGroup("MAIN", &mainGroup);
    if (r != FMOD_OK) return false;
    // eventually should be automated 
    r = SoundSystem->createChannelGroup("BGM", &subGroups["BGM"]);
    if (r != FMOD_OK) return false;
    r = SoundSystem->createChannelGroup("SFX", &subGroups["SFX"]);
    if (r != FMOD_OK) return false;
    r = SoundSystem->createChannelGroup("PERSISTENT", &subGroups["PERSISTENT"]);
    if (r != FMOD_OK) return false;

    std::for_each(subGroups.begin(), subGroups.end(), 
        [this](const std::pair<const std::string,FMOD::ChannelGroup*>& p) {mainGroup->addGroup(p.second); });

    initialized = true;
    return true;
}

// called every frame to let FMOD process active sounds
void AudioManager::Update() {
    if (initialized && SoundSystem) {
        SoundSystem->update();
    }
}


void AudioManager::Free() {
    if (!initialized || !SoundSystem) return;
    SoundSystem->close();
    SoundSystem->release();
    SoundSystem = nullptr;
    initialized = false;
}


AudioObj AudioManager::LoadAudio(const std::string& name, std::string type, bool loop, bool stream) {
    if (!SoundSystem) return {};

    FMOD::Sound* pSound = nullptr;
    FMOD::ChannelGroup* group = nullptr;
    FMOD_MODE mode = FMOD_DEFAULT;

#ifdef PLATFORM_ANDROID
    stream = false; // Android streaming does not work well, so it will always be force disabled.
#endif
    
    if (subGroups.find(type) == subGroups.end()) {
        group = subGroups["SFX"];
    }
    else {
        group = subGroups[type];
    }

    if (loop)   mode |= FMOD_LOOP_NORMAL;
    else        mode |= FMOD_LOOP_OFF;

    if (stream) mode |= FMOD_CREATESTREAM;

    FMOD_RESULT result;



#ifdef PLATFORM_WINDOWS
    result = SoundSystem->createSound(name.c_str(), mode, nullptr, &pSound);
#endif

#ifdef PLATFORM_ANDROID
    // On Android, FMOD cannot access the filesystem directly, so load audio data into memory
    std::stringstream streamData = CEO::Instance().GetManager<FileManager>()->ReadFile(name, std::ios_base::binary | std::ios_base::out);

    FMOD_CREATESOUNDEXINFO soundInfo{};
    soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
    std::string data = streamData.str();
    soundInfo.length = data.size(); // Tell FMOD size of the buffer in memory so it can create the sound correctly

    result = SoundSystem->createSound(data.c_str(), mode | FMOD_OPENMEMORY, &soundInfo, &pSound);
#endif

    if (result != FMOD_OK) {
        // log error
        // to replace with proper logging later
        return {};
    }

    return AudioObj(pSound, SoundSystem, group, type, name);
}

void AudioManager::FreeAudio(AudioObj& obj) {
    obj.Release();
}

void AudioManager::PauseAll(bool b) {
    mainGroup->setPaused(b);
}

void AudioManager::PauseGroup(const std::string& type, bool b) {
    subGroups[type]->setPaused(b);
}

void AudioManager::StopAll() {
    mainGroup->stop(); 
}

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
bool AudioManager::FadeInBGM(FMOD::Channel* channel,float duration, float dt) {
    fadeInTimer += dt; // your delta time
    float t = std::clamp(fadeInTimer / duration, 0.0f, 1.0f);

    float volume = LerpClamped(0.0f, 1.0f, t);
    float pitch = GetGroupPitch("BGM");
    bool mute = GetGroupMute("BGM");    
   
    (void)pitch;
    (void)mute;

    channel->setVolume(volume);
    //SetGroupVars("BGM", volume, pitch, mute);

    if (t >= 1.0f) {
        fadeInTimer = 0.f; // reset for next time
        return true; // fade complete
    }
    return false;
}

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
bool AudioManager::FadeOutBGM(FMOD::Channel* channel,float duration,float dt) {
    fadeOutTimer += dt; // your delta time
    float t = std::clamp(fadeOutTimer / duration, 0.0f, 1.0f);

    float volume = LerpClamped(1.0f, 0.0f, t);
    float pitch = GetGroupPitch("BGM");
    bool mute = GetGroupMute("BGM");

    (void)pitch;
    (void)mute;

    channel->setVolume(volume);
    //SetGroupVars("BGM", volume, pitch, mute);


    if (t >= 1.0f) {
        fadeOutTimer = 0.f; // reset for next time
        return true; // fade complete
    }
    return false;
}

void AudioManager::SetGroupVars(const std::string& name, float vol, float pitch, bool mute) {
    if (subGroups.find(name) != subGroups.end()) {
        subGroups[name]->setVolume(vol);
        subGroups[name]->setPitch(pitch);
        subGroups[name]->setMute(mute);
    }
}
float AudioManager::GetGroupVol(const std::string& name) {
    if (subGroups.find(name) != subGroups.end()) {
        float f;
        subGroups[name]->getVolume(&f);
        return f;
    }
    else return false;
}

float AudioManager::GetGroupPitch(const std::string& name) {
    if (subGroups.find(name) != subGroups.end()) {
        float f;
        subGroups[name]->getPitch(&f);
        return f;
    }
    else return false;
}

bool AudioManager::GetGroupMute(const std::string& name) {
    if (subGroups.find(name) != subGroups.end()) {
        bool b;
        subGroups[name]->getMute(&b);
        return b;
    }
    else return false;
}


