/*!
@file       ExitButtonScript.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Implementation of exit button functionality. Sets up the button's click
event to trigger application exit.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class ExitButtonScript : public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<ExitButtonScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ExitButtonScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ExitButtonScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ExitButtonScript)
)