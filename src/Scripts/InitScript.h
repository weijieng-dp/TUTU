/**___________________________________________________________________________/
@file          InitScript.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Initalizes the statemachines for the various entities
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "EnemyStates.h"
#include "EnemyConfig.h"
#include "FlipState.h"


class InitScript : public ScriptInstance
{
private:

public:
	void BindFrom() { 
        GetComponent<InitScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<InitScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<InitScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry&) {}
    void OnUpdate(Registry&, float, bool) {}
    void OnFixedUpdate(Registry&, float, bool) {}

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(InitScript)
)

