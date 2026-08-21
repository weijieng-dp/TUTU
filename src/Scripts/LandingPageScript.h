/**___________________________________________________________________________/
@file       LandingPageScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       05/03/2026   (DD/MM/YYYY)
@brief      Script attached to the landing page scene. Handles BGM playback
            on start, triggers a fade to black on mouse click to transition
            to the next scene, and provides a debug shortcut to fast-forward
            achievement tracking and jump to the achievement scene.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class LandingPageScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<LandingPageScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<LandingPageScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<LandingPageScript>(*CEO::Get<Registry>()) = *this; };

    /*!
     * \brief Called once when the script starts. Queues the main menu BGM.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Handles input for fade transition and
     *        debug achievement fast-forward shortcut.
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

    GameObject fadeGameObject;      // GameObject containing the FadeScript used to transition to the next scene
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(LandingPageScript),
    field(fadeGameObject)
)