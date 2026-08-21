/*!
@file       ChangeScene.h
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple script to change scene on button press

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/ResourceManager.h"
#include <string>
class ChangeScene : public ScriptInstance
{
    ResourceManager* resourceManager;
    ComponentRegistry* compRegistry;
    Registry* reg;
public:
	void BindFrom() { 
        GetComponent<ChangeScene>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<ChangeScene>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<ChangeScene>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);

    /*!
    * \brief
    *    change current scene to stored scene name
    */
    void SceneChange(/*Registry& registry*/);

    std::string sceneName;

	REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(ChangeScene),
    field(sceneName)
)