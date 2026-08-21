/**___________________________________________________________________________/
@file          EnemyInitScript.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Initalizes the Enemies
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "FlipState.h"


class EnemyInitScript : public ScriptInstance
{
private:

public:
    void BindFrom() {
        GetComponent<EnemyInitScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<EnemyInitScript>(*CEO::Get<Registry>());
    };
    void BindTo() { *GetComponent<EnemyInitScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry) {

        EnemyConfig::Init(); // Initialize configurations needed by enemy scripts
        StateComponent* sc = registry.GetComponent<StateComponent>(entity);

        EnemyComponent* ec = registry.GetComponent<EnemyComponent>(entity);
        switch (ec->type) {
        case EnemyComponent::GIRL:
            sc->stateMachines.push_back(std::make_shared<GirlStateMachine>());
            sc->stateMachines.back()->OnStart(registry, entity);
            sc->stateMachines.push_back(std::make_shared<FlipStateMachine>());
            sc->stateMachines.back()->OnStart(registry, entity);
            break;
        case EnemyComponent::EYEBALL:
            EnemyConfig::EyeInit(ec, entity);
            break;
        case EnemyComponent::ONI:
            EnemyConfig::OniInit(ec, entity);
            break;
        case EnemyComponent::GUY:
            EnemyConfig::GuyInit(ec, entity);
            break;
        case EnemyComponent::LARVA:
            EnemyConfig::LarvaInit(ec, entity);
            break;
        }
    }
    void OnUpdate(Registry&, float, bool) {}
    void OnFixedUpdate(Registry&, float, bool) {}

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(EnemyInitScript)
)

