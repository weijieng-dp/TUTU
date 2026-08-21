/**___________________________________________________________________________/
@file          EnemyCollisionScript.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Enemy scripting header file
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "PlayerControllerScript.h"

class EnemyCollisionScript : public ScriptInstance
{
private:

public:
	void BindFrom() { 
        GetComponent<EnemyCollisionScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<EnemyCollisionScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<EnemyCollisionScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);
    void OnTriggerStay(const Collider& other);
    void OnCollisionEnter(const Collider& other);

    void OnDamageTaken(int dmg);
    void OnDeath();

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(EnemyCollisionScript)
)