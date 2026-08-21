/*!
@file       audio.cpp
@author     Zhang Mingyang (mingyang.zhang) (100%)
@date       05/11/2025

This file implements the [AudioObj] class, which manages sound playback
through FMOD. It includes functionality for controlling sound states,
volume adjustments, and playback looping. The class interfaces directly
with FMOD channels and sound objects to manage runtime audio behavior.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "pch.h"
#include "audio.h"

AudioObj& AudioObj::operator=(AudioObj&& other) noexcept {
    if (this != &other) {
        AudioObj temp(std::move(other));  //steal other's data
        Swap(temp); //exchange with temp
    }
    return *this;
}

void AudioObj::Swap(AudioObj& other) noexcept {
    std::swap(sound, other.sound);
    std::swap(system, other.system);
    std::swap(isActive, other.isActive);
    std::swap(parent, other.parent);
    std::swap(type, other.type);
    std::swap(path, other.path);
}

AudioObj::AudioObj(AudioObj&& other) noexcept {
    sound = other.sound;
    system = other.system;
    isActive = other.isActive;
    parent = other.parent;
    type = other.type;
    path = other.path;

    other.sound = nullptr;
    other.system = nullptr;
    other.isActive = false;
}

FMOD::Channel* AudioObj::Play(float volume, float pitch,float panning){
    if (!isActive || !system || !sound) return nullptr;


    FMOD::Channel* ch = nullptr;
    if (system->playSound(sound, nullptr, true, &ch) != FMOD_OK){
        return nullptr;
    }

    float vol,pit;
    parent->getVolume(&vol);
    parent->getPitch(&pit);

    ch->setChannelGroup(parent);
    ch->setVolume(volume * vol);
    ch->setPitch(pitch * pit);
    ch->setPan(panning);
    ch->setPaused(false);

    // maybe a wrapper for channel in the future,
    // for a slight edge case. I dont see it as too impt rn

    return ch;
}

void AudioObj::Release() {
    if (isActive && sound) {
        sound->release();
    }
    sound = nullptr;
    system = nullptr;
    isActive = false;
}

