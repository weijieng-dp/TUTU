/**___________________________________________________________________________/
@file       FadeScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       24/02/2026   (DD/MM/YYYY)
@brief      Script that handles screen fade transitions between scenes.
            Supports fading to and from black, optional BGM crossfading,
            and video playback integration where the fade out is triggered
            once the video has finished playing.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/SceneManager.h"
#include "../CoreLib/GameObjects.h"


class FadeScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<FadeScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<FadeScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<FadeScript>(*CEO::Get<Registry>()) = *this; };

    bool fade = true;                    // whether the fade animation is currently active
    bool fadeToBlack = false;            // whether the current fade direction is to black (true) or from black (false)
    bool isFading = false;               // whether a fade sequence is currently in progress
    bool queueFadeAction = false;        // whether a fade out should be triggered after the current fade in completes
    float fadeTimer = 0.0f;              // tracks elapsed time for the current fade animation
    float fadeDuration = 0.5f;           // duration in seconds of the fade animation
    std::string sceneToTransition = "";  // name of the scene to load after fade to black completes
    std::string musicToTransition = "";  // filename of the BGM to crossfade to during the transition
    GameObject videoPlayer;              // optional video player GameObject that triggers the fade when playback ends
    Registry::Entity fakeButton{};       // fake button instantiated object
    bool videoFound{ false };            // whether a valid video player was found on start

    /*!
     * \brief Called once when the script starts. Initializes fade state and
     *        checks if a video player is assigned.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Handles video-triggered fading and calls
     *        ToggleFade to update the fade animation each frame.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the update.
     */
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
     * \brief Called every fixed timestep. Currently unused.
     * \param[in] registry   - The ECS registry.
     * \param[in] dt         - Fixed delta time in seconds.
     * \param[in] firstframe - Whether this is the first frame of the fixed update.
     */
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    
    /*!
     * \brief Updates the sprite alpha each frame to produce a fade in or out effect.
     *        Triggers scene transition once fade to black is complete.
     * \param[in] toBlack  - Whether to fade to black (true) or from black (false).
     * \param[in] dt       - Delta time in seconds.
     * \param[in] registry - The ECS registry.
     */
    void ToggleFade(bool toBlack, float dt, Registry& registry);

    /*!
     * \brief Initiates a fade to black, optionally crossfading the BGM
     *        if a music transition is specified.
     */
    void FadeToBlack();
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(FadeScript),
    field(fadeDuration),
    field(sceneToTransition),
    field(musicToTransition),
    field(videoPlayer)
)