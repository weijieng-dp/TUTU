/**___________________________________________________________________________/
@file       ParticleScript.h
@author     d.lorenzoyongoyong@digipen.edu
@date       03/03/2026   (DD/MM/YYYY)
@brief      Script that synchronizes the active state of a GameObject with
            its ParticleEmitterComponent's enabled state, ensuring the object
            is only active while the particle emitter is running.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class ParticleScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<ParticleScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ParticleScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ParticleScript>(*CEO::Get<Registry>()) = *this; };

    /*!
     * \brief Called once when the script starts. Currently unused.
     * \param[in] registry - The ECS registry.
     */
    void OnStart(Registry& registry);

    /*!
     * \brief Called every frame. Mirrors the ParticleEmitterComponent's enabled
     *        state onto the GameObject's active state.
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
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ParticleScript)
)