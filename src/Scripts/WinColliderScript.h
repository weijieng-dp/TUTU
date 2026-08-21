/**___________________________________________________________________________/
@file       WinColliderScript.h
@author     d.lorenzoyongoyong@digipen.edu  80%
@co-author	Tan Jun Jie (t.junjie)			20%
@date       01/02/2026   (DD/MM/YYYY)
@brief      Script attached to the win collider trigger zone. Detects when
            the player enters the win trigger, starts spiral animation, and
            transitions to Final Wave scene upon completion of animation.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class WinColliderScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<WinColliderScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<WinColliderScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<WinColliderScript>(*CEO::Get<Registry>()) = *this; };

    /*!
    * \brief Called once when the script starts. Currently unused.
    * \param[in] registry - The ECS registry.
    */
    void OnStart(Registry& registry);

    /*!
    * \brief Called every frame. Currently unused.
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
    * \brief Called when another collider enters this trigger zone. If the
    *        entering entity is the player, updates the win achievement tracker,
    *        flags the run as a win, and pushes the win scene.
    * \param[in] other - The collider that entered the trigger.
    */
    void OnTriggerEnter(const Collider& other);

    /*!
    * \brief Start the spiral animation.
    * \param[in, out] registry - The entity registry.
    * \param[in] player        - The entity id of the player entity.
    */
    void StartSpiral(Registry& registry, EntityRegistry::Entity player);

    /*!
    * \brief The spiral animation loop.
    * \param[in, out] registry  - The entity registry.
    * \param[in] dt             - The delta time.
    */
    void PlayerSpiral(Registry& registry, float dt);

    GameObject fadeBg;
   
    REFLECTABLE_PROPERTIES;
private:
    EntityRegistry::Entity playerEnt{};     // the player entity id

    Vec2 spiralCenter{};                    // the center for the player to spiral into
    Vec2 initialScale{};                    // starting scale of player
    float initialRadius{};                  // starting spiral radius
    float initialAngle{};                   // starting spiral angle
    float spiralTimer{};                    // timer to track spiral animation
    float spiralDuration{ 1.f };            // spiral animation duration
    bool isSpiralActive{ false };           // flag for whether spiral animation is active
};
REFL_AUTO(
    type(WinColliderScript)
)