/*!
@file       PlayerController.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Declares the PlayerController script class responsible for handling
            player input and movement. The controller reads keyboard input and
            applies translation to the entities TransformComponent each frame.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

/*!
* \brief
*	Handles player input and movement within the ECS. The PlayerController
*	script reads keyboard inputs (WASD) to move the entity and supports both
*	frame-based and fixed timestep updates.
*/
class PlayerControllerScript : public ScriptInstance , public IStateMachine
{
public:
    float movementSpeed{};
    Vec2 movementDirection{};
    int playerMaxHealth{};
    int playerCurrentHealth{};

    bool lerpAddition{false};
    float iFrame{ 1.5f }, timeSinceDamage{ iFrame }, lerpingvalue{ 0 }, blinktime{0}, lerpMultiplier{0};

    float idleTimer = 0.0f;

    std::vector<AudioObj*> footsteps;
    bool changeAnim = false;
    bool deathconfirmed = false;

	TransformComponent* playerTransform = nullptr;
    PhysicsComponent* playerVelocity = nullptr;
    AnimatorComponent* playerAnimator = nullptr;
    SpriteRendererComponent* spriterenderer = nullptr;

    Animation* PlayerIdle = nullptr;
    Animation* PlayerRun = nullptr;
    Animation* PlayerDamage = nullptr;
    Animation* PlayerDeath = nullptr;

    GameObject TrialPath{};

    GameObject HealthbarPrefab{};
    GameObject HealthbarGO{};

    GameObject gameOverBGM{};
    
	void BindFrom() { 
        GetComponent<PlayerControllerScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<PlayerControllerScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<PlayerControllerScript>(*CEO::Get<Registry>()) = *this; };


    void SetPlayerHealth();

    void SetAttackStat();
    /*!
    * \brief
    *	Called once when the script is initialized. Used for setup logic such as
    *	logging or initializing internal state.
    * \param
    *	registry - ECS registry instance used to access entity components
    */
    void OnStart(Registry& registry);

	/*!
    * \brief
    *	Called every frame to process player input and apply movement to the
    *	entities TransformComponent.
    * \param
    *	registry - ECS registry instance used to access entity components
    * \param
    *	dt - delta time for frame update
    * \param
    *	firstframe - true if this is the first frame after initialization
    */
    void OnUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *	Called at a fixed timestep for physics or deterministic updates.
    *	This implementation is currently empty but can be extended.
    * \param
    *	registry - ECS registry instance used to access entity components
    * \param
    *	dt - fixed timestep delta
    * \param
    *	firstframe - true if this is the first fixed update after startup
    */
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    void TakeDamage(int);

    void OnDamageTaken(int);

    void OnDeath();

    void OnTriggerExit(const Collider& other);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(

    /*!
    * \brief
    *	Registers PlayerController with the reflection system for runtime access.
    */
    type(PlayerControllerScript),
    field(movementSpeed),
    field (movementDirection),
    field(TrialPath),
    field(blinktime),
    field(lerpMultiplier),
    field(HealthbarPrefab),
    field(gameOverBGM),
    field(idleTimer)
)