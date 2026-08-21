/*!
@file       ItemIconScript.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Defines the ItemIconManager class. Handles UI display for 
            displaying items collected in the game

Copyright (C) 2025 DigiPen Institute of Technology.
All rights reserved.
*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

class ItemIconManager: public ScriptInstance
{
private:
    std::vector<GameObject> itemGameObjects;
    GameObject itemIconPrefab{"ItemIcon"};
public:
	void BindFrom() { 
        GetComponent<ItemIconManager>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ItemIconManager>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ItemIconManager>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ItemIconManager)
)