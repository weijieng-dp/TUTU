/**___________________________________________________________________________/
@file          SimpleAudioScript.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Basic audio scripting used to test the audio object and dragdrop functionality

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"

class SimpleAudioScript : public ScriptInstance
{
private:

public:
	void BindFrom() { 
        GetComponent<SimpleAudioScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<SimpleAudioScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<SimpleAudioScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry, float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(SimpleAudioScript)
)