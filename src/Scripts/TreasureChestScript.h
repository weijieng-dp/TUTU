/*!
@file       TreasureChestScript.h
@author     Kaeden Tan (kaedenjiawei.tan) (90%)
@author     Tan Jun Jie (kaedenjiawei.tan) (10%)
@date		09/03/2026 (DD/MM/YYYY)
@brief      A script that handles the treasure chest in Treasure tile.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/ItemManager.h"

class TreasureChestScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<TreasureChestScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<TreasureChestScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<TreasureChestScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    void OnTriggerEnter(const Collider& other);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(TreasureChestScript)
)