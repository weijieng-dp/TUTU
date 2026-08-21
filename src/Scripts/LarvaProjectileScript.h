
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class LarvaProjectileScript : public ScriptInstance
{
    Vec2 deccel{};
public:
    void BindFrom() {
        GetComponent<LarvaProjectileScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<LarvaProjectileScript>(*CEO::Get<Registry>());
    };
    void BindTo() { *GetComponent<LarvaProjectileScript>(*CEO::Get<Registry>()) = *this; };


    void OnStart(Registry& registry);

    void OnUpdate(Registry& registry, float dt, bool firstframe);

    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(LarvaProjectileScript)
)