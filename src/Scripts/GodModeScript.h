/*!
@file       GodModeScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Declares the GodModeScript class. Toggles god mode via button click and updates button sprite accordingly.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class GodModeScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<GodModeScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<GodModeScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<GodModeScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    TextureObj* on{ &CEO::Get<ResourceManager>()->GetErrorTex()};
    TextureObj* off{ &CEO::Get<ResourceManager>()->GetErrorTex() };

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(GodModeScript),
	field(on),
	field(off)
)