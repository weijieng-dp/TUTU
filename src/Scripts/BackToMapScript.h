/**___________________________________________________________________________/
@file           BackToMapScript.h
@author         Tan Jun Jie (t.junjie) (100%)
@date           03/02/2026 (DD/MM/YYYY)
@brief          Handles buttons in Game.scene, mainly back tot map button that
                brings player back to map scene upon clearing of a room.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"


class BackToMapScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<BackToMapScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<BackToMapScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<BackToMapScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    GameObject backToMapBtn;
    GameObject pauseBtn;
    GameObject fadeBG;
    float timer = 0.0f;
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(BackToMapScript),
    field(fadeBG)
)