/**___________________________________________________________________________/
@file       WaveTransitionScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       01/03/2026   (DD/MM/YYYY)
@brief      Script that handles the transition between dungeon and wave scene.
            Waits for a specified duration, then triggers a fade-to-black
            effect on a target FadeScript object.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class WaveTransitionScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<WaveTransitionScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<WaveTransitionScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<WaveTransitionScript>(*CEO::Get<Registry>()) = *this; };

    /*!
	 * \brief Called once when the script starts. Currently does not perform any initialization.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Increments a timer and triggers the fade
     *        effect once the duration has elapsed.
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

    float timer{ 0.0f };        // elapsed time since the transition started
    float duration{ 2.0f };     // seconds to wait before starting the fade
	GameObject fadeObj;         // GameObject that holds the FadeScript component
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(WaveTransitionScript),
    field(fadeObj)
)