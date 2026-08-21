/*!
@file       UpdateStatViewScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Defines the UpdateStatViewScript class. Displays current player stats
            (movement speed multiplier, attack speed, projectile damage) as a UI text element.
*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class UpdateStatViewScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<UpdateStatViewScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<UpdateStatViewScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<UpdateStatViewScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(UpdateStatViewScript)
)