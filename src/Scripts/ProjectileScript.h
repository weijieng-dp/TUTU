/*!
@file       ProjectileScript.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Script to handle projectile behavior including movement, lifespan management,
and damage dealing on collision.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

class ProjectileScript: public ScriptInstance
{
public:
    GameObject auraChild;
    GameObject poisonTrail;
    GameObject slowTrail;
    GameObject freezeTrail;

    int damage{ 1 };
    float lifespan{ 2 };        // how long projectile should stay alive
    float timeAlive{};          // how long projectile has been alive
    Vec2 direction{};
    std::string prefabName{};   // Used for indexing the gameObjectPool

    bool canSpawnSplit {true};
    Registry::Entity lastHit{0};


	void BindFrom() { 
        GetComponent<ProjectileScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ProjectileScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ProjectileScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    void OnTriggerEnter(const Collider& other);
    void OnTriggerExit(const Collider& other);

    void ReturnToPool(Registry&);
	void SpawnSplit(Registry& r, Registry::Entity lastHitEntity=0);
    void ProjectileInteractions(Registry*, Collider const&);

    // pool that spawns particle on projectile destroyed
    GameObject particlePool{};

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ProjectileScript),
    field(auraChild),
	field(poisonTrail),
    field(slowTrail),
    field(freezeTrail)
)