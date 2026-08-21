/**___________________________________________________________________________/
@file       OnboardingScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       08/03/2026   (DD/MM/YYYY)
@brief      Script that handles the onboarding tutorial overlay displayed
            on the player's first run. Cycles through a series of instructional
            images on click and hides itself once all slides have been shown.
            Skipped automatically on subsequent runs.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class OnboardingScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<OnboardingScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<OnboardingScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<OnboardingScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief Called once when the script starts. Skips onboarding if it is
    *        not the player's first run, otherwise sets up the click callback
    *        and marks first play as complete.
    * \param[in] registry - The ECS registry.
    */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Increments the timer used to debounce clicks.
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
     * \brief Advances to the next onboarding slide. Ignores clicks within
     *        the first 0.5 seconds to prevent accidental skips. Hides the
     *        onboarding overlay once all slides have been shown.
     */
    void Next();

    float timer{};          // tracks elapsed time since last slide change for click debouncing
    int index{};            // current onboarding slide index
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(OnboardingScript)
)