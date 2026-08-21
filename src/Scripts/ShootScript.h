/**___________________________________________________________________________/
@file          ShootScript.h
@author        t.junjie@digipen.edu
@co-author     yukang.ou@digipen.edu
@date          9/29/2025

Script to manage shooting.
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"
#include "ProjectileScript.h"

class ShootScript : public ScriptInstance
{
private:
    GameObject gameObjectPool;
    int projectilesShotThisBurst{};
	float burstAttackSpeed{ 20 };
    bool shooting{ false };
public:
	void BindFrom() { 
        GetComponent<ShootScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ShootScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ShootScript>(*CEO::Get<Registry>()) = *this; };


    float timeSinceLastShot{};

    GameObject projectilePrefab;
    GameObject particlePrefab;

    bool TryShoot(Registry& registry, Vec2 dir);
    ProjectileScript* Shoot(
        Registry& registry,
        Vec2 shotDir,
        float velocity,
        float lifetime,
        int damage,
        float size,
        Mat3 rotation = Mat3());

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ShootScript),
	field(timeSinceLastShot),
	field(projectilePrefab),
    field(particlePrefab)
)