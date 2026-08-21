/*!
@file       PostProcessToggleScript.h
@author     Ou Yukang (yukang.ou)
@date       25/03/2026
@brief		UI script to toggle post processing

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class PostProcessToggleScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<PostProcessToggleScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<PostProcessToggleScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<PostProcessToggleScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    bool isOn{true};
    void ToggleState();
    TextureObj* onTex = &CEO::Get<ResourceManager>()->GetErrorTex();
    TextureObj* offTex = &CEO::Get<ResourceManager>()->GetErrorTex();
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(PostProcessToggleScript),
    field(onTex),
    field(offTex)

)