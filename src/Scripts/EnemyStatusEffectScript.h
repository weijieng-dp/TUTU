
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/StatsManager.h"
#include "FlashingVFX.h"
#include "HealthScript.h"

class EnemyStatusEffectScript: public ScriptInstance
{
private:
    GameObject objPool;

    EnemyComponent* enemyComp{nullptr};
    HealthScript* hp{ nullptr };
    PhysicsComponent* physics{ nullptr };

    FlashingVFX* vfxScript{ nullptr };
    double initialMS{0};

    Color originalColor;
    float originalFlashDuration{};

    Color poisonColor = Color{ 0.f, 1.f, 0.f, 1.f };
    Color slowColor = Color{ 0.5f, 0.5f, 0.5f, 1.f };
    Color freezeColor = Color{ 0.5f, 0.8f, 1.f, 1.f };
public:
    void BindFrom() {
        GetComponent<EnemyStatusEffectScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<EnemyStatusEffectScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<EnemyStatusEffectScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    PoisonEffect poison{ 0,0 };
    SlowEffect slow{ 0,0 };
    FreezeEffect freeze{ 0.f,0.f};
    KnockbackEffect knockback{ 0.f, 0.f, Vec2{0.f, 0.f} };

    GameObject freezeParticle{"FreezeParticle1"};
    GameObject poisonParticle{"PoisonParticle"};

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(EnemyStatusEffectScript),
    field(freezeParticle),
    field(poisonParticle)
)