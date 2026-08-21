#pragma once
/*!
@file          StackTestScript.h
@author        j.junbo@digipen.edu (100%)
@date          2/4/2026

Basic testing script for new scenemanager changes

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/SceneManager.h"

class StackTestScript : public ScriptInstance {
public:
    void BindFrom() {
        GetComponent<StackTestScript>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<StackTestScript>(*CEO::Get<Registry>());
    };
    void BindTo() { *GetComponent<StackTestScript>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry&) {}
    void OnUpdate(Registry&, float, bool) {
#ifdef PLATFORM_WINDOWS
        if (Input::IsKeyPressed(GLFW_KEY_1)) {
            CEO::Instance().GetManager<SceneManager>()->QueueSceneAction("stacktest_2", SceneManager::CHANGE);
        }
        else if (Input::IsKeyPressed(GLFW_KEY_2)) {
            CEO::Instance().GetManager<SceneManager>()->QueueSceneAction("stacktest_2", SceneManager::PUSH);
        }
        else if (Input::IsKeyPressed(GLFW_KEY_3)) {
            CEO::Instance().GetManager<SceneManager>()->QueueSceneAction("", SceneManager::POP);
        }
#endif

    }
    void OnFixedUpdate(Registry&, float, bool) {}

    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type(StackTestScript)
)