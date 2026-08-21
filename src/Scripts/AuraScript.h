/*!
@file       AuraScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Defines the AuraScript class. Handles damage application and aura-based
            projectile interactions when the aura collider triggers with other entities.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

class AuraScript: public ScriptInstance
{
public:
    GameObject parentProjectile;

	void BindFrom() { 
        GetComponent<AuraScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<AuraScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<AuraScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    
    void OnTriggerEnter(const Collider& other);
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(AuraScript),
    field(parentProjectile)
)