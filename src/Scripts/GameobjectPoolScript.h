/*!
@file       GameobjectPoolScript.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple game object pool script that recycles the use of 1 type of prefab

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib//GameObjects.h"
#include <vector>
#include <unordered_map>

class GameobjectPoolScript: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<GameobjectPoolScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<GameobjectPoolScript>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<GameobjectPoolScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
    
    /*!
    * \brief
    *    set prefab to instance when there are no available gameobjects in pool
    */
    void AddPrefab(GameObject prefab);
    /*!
    * \brief
    *    Get free gameobject from pool or instantiate a new one
    */
    GameObject GetGameobject(const std::string& prefabName);
private:
    std::unordered_map<std::string, std::vector<Registry::Entity>> poolMap;
    std::unordered_map<std::string, GameObject> prefabMap;

public:
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(GameobjectPoolScript)
)