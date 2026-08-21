/**___________________________________________________________________________/
@file          EyeCollisionScript.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Enemy scripting header file
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "PlayerControllerScript.h"

class EyeCollisionScript : public ScriptInstance
{
private:

public:
	void BindFrom() { 
        GetComponent<EyeCollisionScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<EyeCollisionScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<EyeCollisionScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry&) {}
    void OnUpdate(Registry&, float, bool) {}
    void OnFixedUpdate(Registry&, float, bool) {}
    void OnTriggerEnter(const Collider& other);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(EyeCollisionScript)
)