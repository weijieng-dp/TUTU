/*!
@file       HealthScript.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Script to represent health on an entity, handles damage taking and provides
interface for callbacks to handle damage taking

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class HealthScript: public ScriptInstance
{
    using OnHitCallback = std::function<void(int)>;
    using OnDeathCallback = std::function<void()>;
public:
	void BindFrom() { 
        GetComponent<HealthScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<HealthScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<HealthScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *    actions to be handled when taking damage automatically callbacks if needed
    */
    void TakeDamage(int damage);
    OnHitCallback onHitCallback{ nullptr };
    OnDeathCallback onDeathCallback{ nullptr };

    int currHealth{1};
    int maxHealth{1};

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(HealthScript),
    field(maxHealth),
    field(currHealth)
)